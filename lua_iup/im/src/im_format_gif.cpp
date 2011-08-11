/** \file
 * \brief GIF - Graphics Interchange Format
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_gif.cpp,v 1.5 2009/08/23 23:57:51 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

static const int InterlacedOffset[4] = { 0, 4, 2, 1 },  /* The way Interlaced image should */
	               InterlacedJumps[4]  = { 8, 8, 4, 2 };  /* be read - offsets and jumps... */

#define GIF_STAMP	  "GIF"	 /* First chars in file - GIF stamp. */
#define GIF_VERSION	"89a"	 /* First chars in file - GIF stamp. */
 
#define GIF_LZ_BITS		12

#define GIF_LZ_MAX_CODE	    4095		/* Biggest code possible in 12 bits. */
#define GIF_FLUSH_OUTPUT		4096    /* Impossible code, to signal flush. */
#define GIF_FIRST_CODE		  4097    /* Impossible code, to signal first. */
#define GIF_NO_SUCH_CODE		4098    /* Impossible code, to signal empty. */

#define GIF_HT_KEY_MASK		0x1FFF		/* 13bits keys */
#define GIF_HT_KEY_NUM_BITS		13		/* 13bits keys */
#define GIF_HT_MAX_KEY		  8191	  /* 13bits - 1, maximal code possible */
#define GIF_HT_SIZE			    8192	  /* 12bits = 4096 or twice as big! */

/*  GIF89 extension function codes                                             */
#define COMMENT_EXT_FUNC_CODE	    0xFE	/* comment */
#define GRAPHICS_EXT_FUNC_CODE    0xF9	/* graphics control */
#define PLAINTEXT_EXT_FUNC_CODE   0x01	/* plaintext */
#define APPLICATION_EXT_FUNC_CODE 0xFF	/* application block */

/* The 32 bits of the integer are divided into two parts for the key & code:   */
/* 1. The code is 12 bits as our compression algorithm is limited to 12bits */
/* 2. The key is 12 bits Prefix code + 8 bit new char or 20 bits.	    */
#define GIF_HT_GET_KEY(l)	(l >> 12)
#define GIF_HT_GET_CODE(l)	(l & 0x0FFF)
#define GIF_HT_PUT_KEY(l)	(l << 12)
#define GIF_HT_PUT_CODE(l)	(l & 0x0FFF)

struct iGIFData
{
  unsigned char global_colors[256 * 3]; /* global color table if any */
  int global_num_colors, /* global color table number of colors */
      offset,            /* image offset */
      step,              /* interlaced step */
      interlaced,        /* image is interlaced or not */
      screen_width,
      screen_height,
      start_offset[512], /* offset of first block */
	    ClearCode,				 /* The CLEAR LZ code. */
    	BitsPerPixel,	     /* Bits per pixel (Codes uses at list this + 1). */
	    EOFCode,				   /* The EOF LZ code. */
	    RunningCode,		   /* The next code algorithm can generate. */
	    RunningBits,       /* The number of bits required to represent RunningCode. */
	    MaxCode1,          /* 1 bigger than max. possible code, in RunningBits bits. */
	    LastCode,		       /* The code before the current code. */
	    CrntCode,				   /* Current algorithm code. */
	    StackPtr,		       /* For character stack (see below). */
	    CrntShiftState;		 /* Number of bits in CrntShiftDWord. */
  unsigned char Buf[256];	                  /* Compressed input is buffered here. */
  unsigned int CrntShiftDWord;             /* For bytes decomposition into codes. */
  unsigned char Stack[GIF_LZ_MAX_CODE];	    /* Decoded pixels are stacked here. */
  unsigned char Suffix[GIF_LZ_MAX_CODE+1];	/* So we can trace the codes. */
  unsigned int Prefix[GIF_LZ_MAX_CODE+1];
  unsigned int HTable[GIF_HT_SIZE];            /* hash table for the compression only, when using LZW */
};

/******************************************************************************
* Routine to generate an HKey for the hashtable out of the given unique key.  *
* The given Key is assumed to be 20 bits as follows: lower 8 bits are the     *
* new postfix character, while the upper 12 bits are the prefix code.	      *
* Because the average hit ratio is only 2 (2 hash references per entry),      *
* evaluating more complex keys (such as twin prime keys) does not worth it!   *
******************************************************************************/
static int iGIFHashKeyItem(unsigned int Item)
{
  return ((Item >> 12) ^ Item) & GIF_HT_KEY_MASK;
}

/******************************************************************************
* Routine to insert a new Item into the HashTable. The data is assumed to be  *
* new one.								      *
******************************************************************************/
static void iGIFInsertHashTable(unsigned int *HTable, unsigned int Key, int Code)
{
  int HKey = iGIFHashKeyItem(Key);

  while (GIF_HT_GET_KEY(HTable[HKey]) != 0xFFFFFL) 
  {
    HKey = (HKey + 1) & GIF_HT_KEY_MASK;
  }

  HTable[HKey] = GIF_HT_PUT_KEY(Key) | GIF_HT_PUT_CODE(Code);
}

/******************************************************************************
* Routine to test if given Key exists in HashTable and if so returns its code *
* Returns the Code if key was found, -1 if not.				      *
******************************************************************************/
static int iGIFExistsHashTable(unsigned int *HTable, unsigned int Key)
{
  int HKey = iGIFHashKeyItem(Key);
  unsigned int HTKey;

  while ((HTKey = GIF_HT_GET_KEY(HTable[HKey])) != 0xFFFFFL) 
  {
    if (Key == HTKey) 
      return GIF_HT_GET_CODE(HTable[HKey]);

    HKey = (HKey + 1) & GIF_HT_KEY_MASK;
  }

  return -1;
}

/******************************************************************************
*   This routines buffers the given characters until 255 characters are ready *
* to be output. If Code is equal to -1 the buffer is flushed (EOF).	      *
*   The buffer is Dumped with first byte as its size, as GIF format requires. *
******************************************************************************/
static int iGIFBufferedOutput(imBinFile* handle, unsigned char *Buf, int c)
{
  if (c == GIF_FLUSH_OUTPUT) 
  {
    /* Flush everything out. */
    if (Buf[0] != 0)
      imBinFileWrite(handle, Buf, Buf[0] + 1, 1);

    /* Mark end of compressed data, by an empty block (see GIF doc): */
    Buf[0] = 0;
    imBinFileWrite(handle, Buf, 1, 1);
  }
  else 
  {
    if (Buf[0] == 255) 
    {
      /* Dump out this buffer - it is full: */
      imBinFileWrite(handle, Buf, Buf[0] + 1, 1);
      Buf[0] = 0;
    }

    Buf[++Buf[0]] = (unsigned char)c;
  }

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

static int iGIFWriteNetscapeApplication(imBinFile* handle, short iterations)
{
  /* record type byte */
  imBinFileWrite(handle, (void*)"!", 1, 1);

  /* block label */
  imBinFileWrite(handle, (void*)"\xFF", 1, 1);

  /* block size */
  imBinFileWrite(handle, (void*)"\x0B", 1, 1);

  /* application identifier + athentication code */
  imBinFileWrite(handle, (void*)"NETSCAPE2.0", 11, 1);

  /* sub block size */
  imBinFileWrite(handle, (void*)"\x3", 1, 1);

  /* ??? */
  imBinFileWrite(handle, (void*)"\x1", 1, 1);

  /* iterations */
  imBinFileWrite(handle, &iterations, 1, 2);

  /* block terminator */
  imBinFileWrite(handle, (void*)"\0", 1, 1);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

/******************************************************************************
*   The LZ compression output routine:					      *
*   This routine is responsable for the compression of the bit stream into    *
* 8 bits (bytes) packets.						      *
******************************************************************************/
static int iGIFCompressOutput(iGIFData* igif, imBinFile* handle, int Code)
{
  int error = IM_ERR_NONE;

  if (Code == GIF_FLUSH_OUTPUT) 
  {
    while (igif->CrntShiftState > 0) 
    {
      /* Get Rid of what is left in DWord, and flush it. */
      error = iGIFBufferedOutput(handle, igif->Buf, igif->CrntShiftDWord & 0xff);
      igif->CrntShiftDWord >>= 8;
      igif->CrntShiftState -= 8;
    }
    igif->CrntShiftState = 0;			   /* For next time. */
    error = iGIFBufferedOutput(handle, igif->Buf, GIF_FLUSH_OUTPUT);
  }
  else 
  {
    igif->CrntShiftDWord |= ((int) Code) << igif->CrntShiftState;
    igif->CrntShiftState += igif->RunningBits;
    while (igif->CrntShiftState >= 8) 
    {
      /* Dump out full bytes: */
      error = iGIFBufferedOutput(handle, igif->Buf, igif->CrntShiftDWord & 0xff);
      igif->CrntShiftDWord >>= 8;
      igif->CrntShiftState -= 8;
    }
  }

  /* If code cannt fit into RunningBits bits, must raise its size. Note */
  /* however that codes above 4095 are used for special signaling.      */
  if (igif->RunningCode >= igif->MaxCode1 && Code <= GIF_LZ_MAX_CODE) 
  {
    igif->MaxCode1 = 1 << ++igif->RunningBits;
  }

  return error;
}

/******************************************************************************
*   The LZ compression routine:						      *
*   This version compress the given buffer Line of length LineLen.	      *
*   This routine can be called few times (one per scan line, for example), in *
* order the complete the whole image.					      *
******************************************************************************/
static int iGIFCompressLine(iGIFData* igif, imBinFile* handle, unsigned char *Line, int LineLen)
{
  int i = 0, CrntCode, NewCode;
  unsigned int NewKey;
  unsigned char Pixel;

  if (igif->CrntCode == GIF_FIRST_CODE)		  /* Its first time! */
    CrntCode = Line[i++];
  else
    CrntCode = igif->CrntCode;     /* Get last code in compression. */

  while (i < LineLen) 
  {			    /* Decode LineLen items. */
    Pixel = Line[i++];		      /* Get next pixel from stream. */
    /* Form a new unique key to search hash table for the code combines  */
    /* CrntCode as Prefix string with Pixel as postfix char.	     */
    NewKey = (((unsigned int) CrntCode) << 8) + Pixel;

    if ((NewCode = iGIFExistsHashTable(igif->HTable, NewKey)) >= 0) 
    {
      /* This Key is already there, or the string is old one, so	     */
      /* simple take new code as our CrntCode:			     */
      CrntCode = NewCode;
    }
    else 
    {
      /* Put it in hash table, output the prefix code, and make our    */
      /* CrntCode equal to Pixel.					     */
      if (iGIFCompressOutput(igif, handle, CrntCode) != IM_ERR_NONE) 
        return IM_ERR_ACCESS;

      CrntCode = Pixel;

      /* If however the HashTable if full, we send a clear first and   */
      /* Clear the hash table.					     */
      if (igif->RunningCode >= GIF_LZ_MAX_CODE) 
      {
        /* Time to do some clearance: */
        if (iGIFCompressOutput(igif, handle, igif->ClearCode) != IM_ERR_NONE) 
          return IM_ERR_ACCESS;

        igif->RunningCode = igif->EOFCode + 1;
        igif->RunningBits = igif->BitsPerPixel + 1;
        igif->MaxCode1 = 1 << igif->RunningBits;
        memset(igif->HTable, 0xFF, GIF_HT_SIZE * sizeof(int));
      }
      else 
      {
        /* Put this unique key with its relative Code in hash table: */
        iGIFInsertHashTable(igif->HTable, NewKey, igif->RunningCode++);
      }
    }
  }

  /* Preserve the current state of the compression algorithm: */
  igif->CrntCode = CrntCode;

  return IM_ERR_NONE;
}

/******************************************************************************
*   This routines read one gif data block at a time and buffers it internally *
* so that the decompression routine could access it.			      *
*   The routine returns the next byte from its internal buffer (or read next  *
* block in if buffer empty).		      *
******************************************************************************/
static int iGIFBufferedInput(imBinFile* handle, unsigned char *Buf, unsigned char *NextByte)
{
  if (Buf[0] == 0) 
  {
    /* Needs to read the next buffer - this one is empty: */
    imBinFileRead(handle, Buf, 1, 1);
    imBinFileRead(handle, &Buf[1], Buf[0], 1);

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;

    *NextByte = Buf[1];
    Buf[1] = 2;	   /* We use now the second place as last char read! */
    Buf[0]--;
  }
  else 
  {
    *NextByte = Buf[Buf[1]++];
    Buf[0]--;
  }

  return IM_ERR_NONE;
}

/******************************************************************************
*   The LZ decompression input routine:					      *
*   This routine is responsable for the decompression of the bit stream from  *
* 8 bits (bytes) packets, into the real codes.				      *
******************************************************************************/
static int iGIFDecompressInput(iGIFData* igif, imBinFile* handle, int *Code)
{
  unsigned char NextByte;
  static unsigned int CodeMasks[] = 
  {
    0x0000, 0x0001, 0x0003, 0x0007,
    0x000f, 0x001f, 0x003f, 0x007f,
    0x00ff, 0x01ff, 0x03ff, 0x07ff,
    0x0fff
  };

  while (igif->CrntShiftState < igif->RunningBits) 
  {
    /* Needs to get more bytes from input stream for next code: */
    if (iGIFBufferedInput(handle, igif->Buf, &NextByte) != IM_ERR_NONE) 
      return IM_ERR_ACCESS;

    igif->CrntShiftDWord |= ((unsigned int) NextByte) << igif->CrntShiftState;
    igif->CrntShiftState += 8;
  }

  *Code = igif->CrntShiftDWord & CodeMasks[igif->RunningBits];

  igif->CrntShiftDWord >>= igif->RunningBits;
  igif->CrntShiftState -= igif->RunningBits;

  /* If code cannt fit into RunningBits bits, must raise its size. Note */
  /* however that codes above 4095 are used for special signaling.      */
  if (++(igif->RunningCode) > igif->MaxCode1 && igif->RunningBits < GIF_LZ_BITS) 
  {
    igif->MaxCode1 <<= 1;
    igif->RunningBits++;
  }

  return IM_ERR_NONE;
}

/******************************************************************************
* Routine to trace the Prefixes linked list until we get a prefix which is    *
* not code, but a pixel value (less than ClearCode). Returns that pixel value.*
* If image is defective, we might loop here forever, so we limit the loops to *
* the maximum possible if image O.k. - LZ_MAX_CODE times.		      *
******************************************************************************/
static int iGIFGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode)
{
  int i = 0;

  while (Code > ClearCode && i++ <= GIF_LZ_MAX_CODE) 
    Code = Prefix[Code];

  return Code;
}

static int iGIFDecompressLine(iGIFData* igif, imBinFile* handle, unsigned char *Line,	int LineLen)
{
  int i = 0, j, CrntCode, EOFCode, ClearCode, CrntPrefix, LastCode, StackPtr;
  unsigned char *Stack, *Suffix;
  unsigned int *Prefix;

  StackPtr = igif->StackPtr;
  Prefix = igif->Prefix;
  Suffix = igif->Suffix;
  Stack = igif->Stack;
  EOFCode = igif->EOFCode;
  ClearCode = igif->ClearCode;
  LastCode = igif->LastCode;

  if (StackPtr != 0) 
  {
    /* Let pop the stack off before continueing to read the gif file: */
    while (StackPtr != 0 && i < LineLen) 
      Line[i++] = Stack[--StackPtr];
  }

  while (i < LineLen) 
  {			    
    /* Decode LineLen items. */
    if (iGIFDecompressInput(igif, handle, &CrntCode))
      return IM_ERR_ACCESS;

    if (CrntCode == EOFCode) 
    {
      /* Note however that usually we will not be here as we will stop */
      /* decoding as soon as we got all the pixel, or EOF code will    */
      /* not be read at all, and DGifGetLine/Pixel clean everything.   */
      if (i != LineLen - 1) 
        return IM_ERR_ACCESS;
  
    i++;
    }
    else if (CrntCode == ClearCode) 
    {
      /* We need to start over again: */
      for (j = 0; j <= GIF_LZ_MAX_CODE; j++) 
        Prefix[j] = GIF_NO_SUCH_CODE;

      igif->RunningCode = igif->EOFCode + 1;
      igif->RunningBits = igif->BitsPerPixel + 1;
      igif->MaxCode1 = 1 << igif->RunningBits;
      LastCode = igif->LastCode = GIF_NO_SUCH_CODE;
    }
    else 
    {
      /* Its regular code - if in pixel range simply add it to output  */
      /* stream, otherwise trace to codes linked list until the prefix */
      /* is in pixel range:					     */
      if (CrntCode < ClearCode) 
      {
        /* This is simple - its pixel scalar, so add it to output:   */
        Line[i++] = (unsigned char)CrntCode;
      }
      else 
      {
        /* Its a code to needed to be traced: trace the linked list  */
        /* until the prefix is a pixel, while pushing the suffix     */
        /* pixels on our stack. If we done, pop the stack in reverse */
        /* (thats what stack is good for!) order to output.	     */
        if (Prefix[CrntCode] == GIF_NO_SUCH_CODE) 
        {
          /* Only allowed if CrntCode is exactly the running code: */
          /* In that case CrntCode = XXXCode, CrntCode or the	     */
          /* prefix code is last code and the suffix char is	     */
          /* exactly the prefix of last code!			     */
          if (CrntCode == igif->RunningCode - 2) 
          {
            CrntPrefix = LastCode;
            Suffix[igif->RunningCode - 2] =
            Stack[StackPtr++] = (unsigned char)iGIFGetPrefixChar(Prefix, LastCode, ClearCode);
          }
          else 
            return IM_ERR_ACCESS;
        }
        else
          CrntPrefix = CrntCode;

        /* Now (if image is O.K.) we should not get an NO_SUCH_CODE  */
        /* During the trace. As we might loop forever, in case of    */
        /* defective image, we count the number of loops we trace    */
        /* and stop if we got LZ_MAX_CODE. obviously we can not      */
        /* loop more than that.					     */
        j = 0;
        while (j++ <= GIF_LZ_MAX_CODE && CrntPrefix > ClearCode && CrntPrefix <= GIF_LZ_MAX_CODE) 
        {
          Stack[StackPtr++] = Suffix[CrntPrefix];
          CrntPrefix = Prefix[CrntPrefix];
        }

        if (j >= GIF_LZ_MAX_CODE || CrntPrefix > GIF_LZ_MAX_CODE) 
            return IM_ERR_ACCESS;

        /* Push the last character on stack: */
        Stack[StackPtr++] = (unsigned char)CrntPrefix;

        /* Now lets pop all the stack into output: */
        while (StackPtr != 0 && i < LineLen)
          Line[i++] = Stack[--StackPtr];
      }

      if (LastCode != GIF_NO_SUCH_CODE) 
      {
        Prefix[igif->RunningCode - 2] = LastCode;

        if (CrntCode == igif->RunningCode - 2) 
        {
          /* Only allowed if CrntCode is exactly the running code: */
          /* In that case CrntCode = XXXCode, CrntCode or the	     */
          /* prefix code is last code and the suffix char is	     */
          /* exactly the prefix of last code!			     */
          Suffix[igif->RunningCode - 2] = (unsigned char)iGIFGetPrefixChar(Prefix, LastCode, ClearCode);
        }
        else 
        {
          Suffix[igif->RunningCode - 2] = (unsigned char)iGIFGetPrefixChar(Prefix, CrntCode, ClearCode);
        }
      }
    
      LastCode = CrntCode;
    }
  }

  igif->LastCode = LastCode;
  igif->StackPtr = StackPtr;

  return IM_ERR_NONE;
}

/*******************************************
*   Skip sub-blocks until terminator found *
********************************************/
static int iGIFSkipSubBlocks(imBinFile* handle)
{
  unsigned char byte_value;
  do
  {
    /* reads the number of bytes of the block or the terminator */
    imBinFileRead(handle, &byte_value, 1, 1);

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;

    /* jump number of bytes, ignores the contents */
    if (byte_value) imBinFileSeekOffset(handle, byte_value);
  }while (byte_value != 0);

  return IM_ERR_NONE;
}

static int iGIFSkipImage(imBinFile* handle, int *image_count, int *terminate)
{
  int found_image = 0;
  unsigned char byte_value;

  *terminate = 0;
  do
  {
    /* reads the record type byte */
    byte_value = 0;
    imBinFileRead(handle, &byte_value, 1, 1);

    switch (byte_value) 
    {
	  case ',': /* image description */
      /* jump 8 bytes */
      imBinFileSeekOffset(handle, 8);

      /* reads the image information byte */
      imBinFileRead(handle, &byte_value, 1, 1);

      if (byte_value & 0x80)
      {
        int bpp = (byte_value & 0x07) + 1;
        int num_colors = 1 << bpp;

        /* skip the color table */
        imBinFileSeekOffset(handle, 3*num_colors);
      }

      /* jump 1 byte (LZW Min Code) */
      imBinFileSeekOffset(handle, 1);

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;

      /* skip sub blocks */
      if (iGIFSkipSubBlocks(handle) != IM_ERR_NONE)
        return IM_ERR_ACCESS;

      /* one more image */
      found_image = 1;
      (*image_count)++;
	    break;
	  case '!': /* extension */
      /* jump 1 byte (label) */
      imBinFileSeekOffset(handle, 1);

      /* skip sub blocks */
      if (iGIFSkipSubBlocks(handle) != IM_ERR_NONE)
        return IM_ERR_ACCESS;
	    break;
	  case ';': /* terminate */
	  default:  /* probably EOF */
      *terminate = 1;
      break;
	  }

  } while (!(*terminate) && (!found_image));

  if (!found_image && *image_count == 0)
    return IM_ERR_FORMAT;

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

static void iGIFReadGraphicsControl(imBinFile* handle, imAttribTable* attrib_table)
{
  unsigned char byte_value;
  unsigned short word_value;

  /* jump 1 bytes (size) */
  imBinFileSeekOffset(handle, 1);

  /* reads the packed descrition */
  imBinFileRead(handle, &byte_value, 1, 1);
  if (imBinFileError(handle))
    return;

  /* user input */
  if (byte_value & 0x02)
    attrib_table->Set("UserInput", IM_BYTE, 1, "\x1");

  /* disposal */
  if (byte_value & 0x1C)
  {
    char* disposal;
    int disp = (byte_value & 0x1C) >> 2;

    switch (disp)
    {
    default:
      disposal = "UNDEF";
      break;
    case 0x01:
      disposal = "LEAVE";
      break;
    case 0x02:
      disposal = "RBACK";
      break;
    case 0x04:
      disposal = "RPREV";
      break;
    }

    attrib_table->Set("Disposal", IM_BYTE, -1, disposal);
  }

  /* delay time */
  imBinFileRead(handle, &word_value, 1, 2);
  if (word_value)
    attrib_table->Set("Delay", IM_USHORT, 1, &word_value);

  /* transparency index */
  if (byte_value & 0x01)
  {
    imBinFileRead(handle, &byte_value, 1, 1);
    attrib_table->Set("TransparencyIndex", IM_BYTE, 1, &byte_value);
  }
  else
    imBinFileSeekOffset(handle, 1);

  /* jump 1 bytes (terminator) */
  imBinFileSeekOffset(handle, 1);
}

static int iGIFReadApplication(imBinFile* handle, imAttribTable* attrib_table)
{
  char identifier[9];

  /* jump 1 byte (size) */
  imBinFileSeekOffset(handle, 1);

  /* reads the application identifier */
  imBinFileRead(handle, identifier, 8, 1);
  if (identifier[7] != 0)
    identifier[8] = 0;

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  if (imStrEqual(identifier, "NETSCAPE"))
  {
    unsigned char authentication[4];
    /* reads the application authentication code */
    imBinFileRead(handle, authentication, 3, 1);
    authentication[3] = 0;

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;

    if (strcmp((char*)authentication, "2.0") == 0)
    {
      unsigned short word_value;

      /* jump 2 bytes (size + 1) */
      imBinFileSeekOffset(handle, 2);

      /* reads the number of iterations */
      imBinFileRead(handle, &word_value, 1, 2);

      attrib_table->Set("Iterations", IM_USHORT, 1, &word_value);

      /* jump 1 byte (terminator) */
      imBinFileSeekOffset(handle, 1);

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;
    }
    else
    {
      /* Skip remaining blocks */
      if (iGIFSkipSubBlocks(handle) != IM_ERR_NONE)
        return IM_ERR_ACCESS;
    }
  }
  else
  {
    /* jump 3 bytes (authentication code) */
    imBinFileSeekOffset(handle, 3);

    /* Skip remaining blocks */
    if (iGIFSkipSubBlocks(handle) != IM_ERR_NONE)
      return IM_ERR_ACCESS;
  }

  return IM_ERR_NONE;
}

static int iGIFReadComment(imBinFile* handle, imAttribTable* attrib_table)
{
  unsigned char byte_value, buffer[255*100] = "", *buffer_ptr;
  int size = 0;

  buffer_ptr = &buffer[0];

  do
  {
    /* reads the number of bytes of the block or the terminator */
    imBinFileRead(handle, &byte_value, 1, 1);

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;

    /* reads data */
    if (byte_value)
    {
      imBinFileRead(handle, buffer_ptr, byte_value, 1);

      if (buffer_ptr[byte_value-1] == 0)
      {
        size += byte_value-1;
        buffer_ptr += byte_value-1;
      }
      else
      {
        size += byte_value;
        buffer_ptr += byte_value;
      }
    }

  }while (byte_value != 0);

  if (buffer[0] != 0)
    attrib_table->Set("Description", IM_BYTE, size, buffer);

  return IM_ERR_NONE;
}

static int iGIFReadExtension(imBinFile* handle, imAttribTable* attrib_table)
{
  unsigned char byte_value;

  /* read block label */
  imBinFileRead(handle, &byte_value, 1, 1);

  if (byte_value == 0xF9)
  {
    /* Graphics Control Extension */
    iGIFReadGraphicsControl(handle, attrib_table);
  }
  else if (byte_value == 0xFE)
  {
    /* Comment Extension */
    if (iGIFReadComment(handle, attrib_table) != IM_ERR_NONE)
      return IM_ERR_ACCESS;
  }
  else if (byte_value == 0xFF)
  {
    /* Application Extension */
    if (iGIFReadApplication(handle, attrib_table) != IM_ERR_NONE)
      return IM_ERR_ACCESS;
  }
  else
  {
    /* skip sub blocks */
    if (iGIFSkipSubBlocks(handle) != IM_ERR_NONE)
      return IM_ERR_ACCESS;
  }

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

static int iGIFWriteComment(imBinFile* handle, unsigned char *buffer, int size)
{
  unsigned char byte_value;

  /* record type byte */
  imBinFileWrite(handle, (void*)"!", 1, 1);

  /* block label */
  imBinFileWrite(handle, (void*)"\xFE", 1, 1);

  while (size > 0)
  {
    if (size > 255)
      byte_value = 255;
    else
      byte_value = (unsigned char)size;

    /* sub block size */
    imBinFileWrite(handle, &byte_value, 1, 1);

    /* sub block data */
    imBinFileWrite(handle, buffer, byte_value, 1);

    buffer += byte_value;
    size -= byte_value;

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;
  }

  /* block terminator */
  imBinFileWrite(handle, (void*)"\0", 1, 1);

  return IM_ERR_NONE;
}

static int iGIFWriteGraphicsControl(imBinFile* handle, imAttribTable* attrib_table)
{
  const void *attrib_user_input, *attrib_disposal, *attrib_delay, *attrib_transparency;
  unsigned char byte_value;

  attrib_user_input = attrib_table->Get("UserInput");
  attrib_disposal = attrib_table->Get("Disposal");
  attrib_delay = attrib_table->Get("Delay");
  attrib_transparency = attrib_table->Get("TransparencyIndex");

  /* Writes the Graphics Control Extension */
  if (attrib_user_input || attrib_disposal || attrib_delay || attrib_transparency)
  {
    unsigned short word_value;

    /* record type byte */
    imBinFileWrite(handle, (void*)"!", 1, 1);

    /* block label */
    imBinFileWrite(handle, (void*)"\xF9", 1, 1);

    /* block size */
    imBinFileWrite(handle, (void*)"\x04", 1, 1);

    byte_value = 0;

    /* user input flag */
    if (attrib_user_input && *(unsigned char*)attrib_user_input == 1)
      byte_value |= 0x02;

    /* transparency flag */
    if (attrib_transparency)
      byte_value |= 0x01;

    /* disposal flag */
    if (attrib_disposal)
    {
      int disp = 0;
      if (imStrEqual((char*)attrib_disposal, "LEAVE"))
        disp = 0x01;
      else if (imStrEqual((char*)attrib_disposal, "RBACK"))
        disp = 0x02;
      else if (imStrEqual((char*)attrib_disposal, "RPREV"))
        disp = 0x04;

      disp = disp << 2;
      byte_value |= disp;
    }

    /* packed */
    imBinFileWrite(handle, &byte_value, 1, 1);

    /* delay time */
    if (attrib_delay)
      word_value = *(unsigned short*)attrib_delay;
    else
      word_value = 0;

    imBinFileWrite(handle, &word_value, 1, 2);

    /* transparency index */
    if (attrib_transparency)
    {
      byte_value = *(unsigned char*)attrib_transparency;
      imBinFileWrite(handle, &byte_value, 1, 1);
    }
    else
      imBinFileWrite(handle, (void*)"\0", 1, 1);

    /* terminator */
    imBinFileWrite(handle, (void*)"\0", 1, 1);

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;
  }

  return IM_ERR_NONE;
}

static const char* iGIFCompTable[1] = 
{
  "LZW"
};

class imFileFormatGIF: public imFileFormatBase
{
  imBinFile* handle;
  iGIFData gif_data;

  int GIFReadImageInfo();
  int GIFWriteImageInfo();

public:
  imFileFormatGIF(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatGIF() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatGIF: public imFormat
{
public:
  imFormatGIF()
    :imFormat("GIF", 
              "Graphics Interchange Format", 
              "*.gif;", 
              iGIFCompTable, 
              1, 
              1)
    {}
  ~imFormatGIF() {}

  imFileFormatBase* Create(void) const { return new imFileFormatGIF(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterGIF(void)
{
  imFormatRegister(new imFormatGIF());
}

int imFileFormatGIF::Open(const char* file_name)
{
  this->handle = imBinFileOpen(file_name);
  if (this->handle == NULL)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  unsigned char sig[4];
  if (!imBinFileRead(this->handle, sig, 3, 1))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  sig[3] = 0;
  if (!imStrEqual((char*)sig, GIF_STAMP))
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  /* ignore version */
  imBinFileSeekOffset(handle, 3);

  strcpy(this->compression, "LZW");

  /* reads screen width and screen height */
  imushort word_value;
  imBinFileRead(handle, &word_value, 1, 2);
  gif_data.screen_width = word_value;

  imBinFileRead(handle, &word_value, 1, 2);
  gif_data.screen_height = word_value;

  /* reads color table information byte */
  imbyte byte_value;
  imBinFileRead(handle, &byte_value, 1, 1);

  /* jump 2 bytes (bgcolor + aspect ratio) */
  imBinFileSeekOffset(handle, 2);

  /* global color table, if exists */
  if (byte_value & 0x80)
  {
    int bpp = (byte_value & 0x07) + 1;
    gif_data.global_num_colors = 1 << bpp;

    /* reads the color palette */
    imBinFileRead(handle, gif_data.global_colors, gif_data.global_num_colors * 3, 1);
  }

  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  /* count number of images */
  int error, terminate;
  this->image_count = 0;
  do
  {
    // store each offset before counting images
    gif_data.start_offset[this->image_count] = imBinFileTell(handle);
    error = iGIFSkipImage(handle, &this->image_count, &terminate);
  } while (!terminate && error == IM_ERR_NONE);

  if (this->image_count == 0 || error != IM_ERR_NONE)
  {
    imBinFileClose(handle);
    return error;
  }

  return IM_ERR_NONE;
}

int imFileFormatGIF::New(const char* file_name)
{
  this->handle = imBinFileNew(file_name);
  if (this->handle == NULL)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  /* writes the GIF STAMP and version - header */
  imBinFileWrite(handle, (void*)GIF_STAMP, 3, 1);   /* identifier */
  imBinFileWrite(handle, (void*)GIF_VERSION, 3, 1); /* format version */

  // File header will be written at the first image

  /* tests if everything was ok */
  if (imBinFileError(handle))
  {
    imBinFileClose(this->handle);
    return IM_ERR_ACCESS;
  }

  strcpy(this->compression, "LZW");
  
  return IM_ERR_NONE;
}

void imFileFormatGIF::Close()
{
  if (this->is_new && !imBinFileError(this->handle))
    imBinFileWrite(this->handle, (void*)";", 1, 1);

  imBinFileClose(this->handle);
}

void* imFileFormatGIF::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatGIF::GIFReadImageInfo()
{
  imbyte byte_value;
  imushort word_value;
  int int_value;

  imAttribTable* attrib_table = AttribTable();

  /* reads the image left position */
  imBinFileRead(handle, &word_value, 1, 2);
  if (word_value)
    attrib_table->Set("XScreen", IM_USHORT, 1, &word_value);

  /* reads the image top position */
  imBinFileRead(handle, &word_value, 1, 2);
  if (word_value)
    attrib_table->Set("YScreen", IM_USHORT, 1, &word_value);

  /* reads the image width */
  imBinFileRead(handle, &word_value, 1, 2);
  this->width = word_value;

  /* reads the image height */
  imBinFileRead(handle, &word_value, 1, 2);
  this->height = word_value;

  /* reads the image information byte */
  imBinFileRead(handle, &byte_value, 1, 1);

  gif_data.interlaced = (byte_value & 0x40)? 1: 0;
  if (gif_data.interlaced)
  {
    int_value = 1;
    attrib_table->Set("Interlaced", IM_INT, 1, &int_value);
  }

  this->file_color_mode = IM_MAP;
  this->file_data_type = IM_BYTE;

  /* local color table */
  int num_colors;
  unsigned char *colors;
  unsigned char local_colors[256 * 3];

  if (byte_value & 0x80)
  {
    int bpp = (byte_value & 0x07) + 1;
    num_colors = 1 << bpp;
    colors = local_colors;

    /* reads the color table */
    imBinFileRead(handle, local_colors, num_colors * 3, 1);
  }
  else if (gif_data.global_num_colors)
  {
    colors = gif_data.global_colors;
    num_colors = gif_data.global_num_colors;
  }
  else
    return IM_ERR_FORMAT;

  long palette[256];
  for (int c = 0; c < num_colors; c++)
  {
    palette[c] = imColorEncode(colors[c*3],
                               colors[c*3+1],
                               colors[c*3+2]);
  }

  imFileSetPalette(this, palette, num_colors);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  this->file_color_mode |= IM_TOPDOWN;

  return IM_ERR_NONE;
}

int imFileFormatGIF::GIFWriteImageInfo()
{
  this->file_data_type = IM_BYTE;
  this->file_color_mode = imColorModeSpace(this->user_color_mode);
  this->file_color_mode |= IM_TOPDOWN;

  imAttribTable* attrib_table = AttribTable();
  const void* attrib = attrib_table->Get("Interlaced");
  if (attrib)
    gif_data.interlaced = *(int*)attrib;

  imBinFileWrite(handle, (void*)",", 1, 1);  /* Image separator character. */

  imushort word_value;

  attrib = attrib_table->Get("XScreen");
  if (attrib)
    word_value = *(unsigned short*)attrib;
  else
    word_value = 0;
  imBinFileWrite(handle, &word_value, 1, 2); /* image left */

  attrib = attrib_table->Get("YScreen");
  if (attrib)
    word_value = *(unsigned short*)attrib;
  else
    word_value = 0;
  imBinFileWrite(handle, &word_value, 1, 2); /* image top */

  word_value = (unsigned short)this->width;
  imBinFileWrite(handle, &word_value, 1, 2); /* image width */
  word_value = (unsigned short)this->height;
  imBinFileWrite(handle, &word_value, 1, 2); /* image height */

  /* local color table */
  imbyte byte_value = 0x80;
  if (gif_data.interlaced)
    byte_value |= 0x40;

  int num_colors = 256;
  if (imColorModeSpace(this->user_color_mode) == IM_MAP)
  {
    int bpp = 0, c = this->palette_count-1;
    while (c) {c = c >> 1;bpp++;} 
    byte_value |= (bpp-1); 
    num_colors = 1 << bpp;
  }
  else
    byte_value |= 0x07; /* 8 bits = 256 grays */

  imBinFileWrite(handle, &byte_value, 1, 1); /* image information */

  /* write color table */
  unsigned char local_colors[256*3];
  for (int c = 0; c < num_colors; c++) // write all data, even not used colors
  {
    unsigned char r, g, b;
    imColorDecode(&r, &g, &b, this->palette[c]);
    local_colors[c*3] = r;
    local_colors[c*3+1] = g;
    local_colors[c*3+2] = b;
  }
  imBinFileWrite(handle, local_colors, num_colors*3, 1);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

int imFileFormatGIF::ReadImageInfo(int index)
{
  imAttribTable* attrib_table = AttribTable();

  /* must clear the attribute list, because it can have multiple images and 
     has many attributes that may exists only for specific images. */
  attrib_table->RemoveAll();
  imFileSetBaseAttributes(this);

  if (gif_data.screen_width) 
  {
    imushort word_value = (imushort)gif_data.screen_width;
    attrib_table->Set("ScreenWidth", IM_USHORT, 1, &word_value);
  }

  if (gif_data.screen_height) 
  {
    imushort word_value = (imushort)gif_data.screen_height;
    attrib_table->Set("ScreenHeight", IM_USHORT, 1, &word_value);
  }

  /* jump to start offset of the image */
  imBinFileSeekTo(handle, gif_data.start_offset[index]);

  int found_image = 0;
  imbyte byte_value;

  int terminate = 0;
  do
  {
    /* reads the record type byte */
    byte_value = 0;
    imBinFileRead(handle, &byte_value, 1, 1);

    switch (byte_value) 
    {
	  case '!': /* 0x21 extension (appears before the image) */
      if (iGIFReadExtension(handle, attrib_table) != IM_ERR_NONE)
        return IM_ERR_ACCESS;
	    break;
	  case ',': /* 0x2C image description and color table */
      if (GIFReadImageInfo() != IM_ERR_NONE)
        return IM_ERR_ACCESS;

      /* we will read only this image for now, so break the loop */
      found_image = 1;
	    break;
	  case ';': /* if terminate before find image return error */
	  default:
      terminate = 1;
      break;
	  }
  } while (!terminate && !found_image);

  if (!found_image)
    return IM_ERR_ACCESS;

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  /* reads the LZW Min code byte */
  imBinFileRead(handle, &byte_value, 1, 1);

  /* now initialize the compression control data */

  gif_data.BitsPerPixel = byte_value;
  gif_data.ClearCode = (1 << byte_value);
  gif_data.EOFCode = gif_data.ClearCode + 1;
  gif_data.RunningCode = gif_data.EOFCode + 1;
  gif_data.RunningBits = byte_value + 1;	 /* Number of bits per code. */
  gif_data.MaxCode1 = 1 << gif_data.RunningBits;     /* Max. code + 1. */
  gif_data.StackPtr = 0;		    /* No pixels on the pixel stack. */
  gif_data.LastCode = GIF_NO_SUCH_CODE;
  gif_data.CrntShiftState = 0;	/* No information in CrntShiftDWord. */
  gif_data.CrntShiftDWord = 0;
  gif_data.Buf[0] = 0;			      /* Input Buffer empty. */

  for (int i = 0; i <= GIF_LZ_MAX_CODE; i++) 
    gif_data.Prefix[i] = GIF_NO_SUCH_CODE;

  gif_data.step = 0;

  return IM_ERR_NONE;
}

int imFileFormatGIF::WriteImageInfo()
{
  this->file_color_mode = imColorModeSpace(this->user_color_mode);
  this->file_color_mode |= IM_TOPDOWN;
  this->file_data_type = this->user_data_type;

  imAttribTable* attrib_table = AttribTable();
  const void* attrib_data;
  int attrib_size;

  if (this->image_count == 0)
  {
    imushort word_value;

    // write file header

    /* logical screen descriptor */
    attrib_data = attrib_table->Get("ScreenWidth");
    if (attrib_data) word_value = *(imushort*)attrib_data;
    else             word_value = (imushort)this->width;
    imBinFileWrite(handle, &word_value, 1, 2);

    attrib_data = attrib_table->Get("ScreenHeight");
    if (attrib_data) word_value = *(imushort*)attrib_data;
    else             word_value = (imushort)this->height;
    imBinFileWrite(handle, &word_value, 1, 2);

    imbyte byte_value = 0;  /* no global color table, 0 colors */
    imBinFileWrite(handle, &byte_value, 1, 1); /* screen information */
    imBinFileWrite(handle, (void*)"\0\0", 2, 1);  /* (bgcolor + aspect ratio) */
  }

  attrib_data = attrib_table->Get("Description", NULL, &attrib_size);
  if (attrib_data)
  {
    if (iGIFWriteComment(handle, (imbyte*)attrib_data, attrib_size) != IM_ERR_NONE)
      return IM_ERR_ACCESS;
  }

  attrib_data = attrib_table->Get("Iterations");
  if (attrib_data)
  {
    if (iGIFWriteNetscapeApplication(handle, *(short*)attrib_data) != IM_ERR_NONE)
      return IM_ERR_ACCESS;
  }

  if (iGIFWriteGraphicsControl(handle, attrib_table) != IM_ERR_NONE)
    return IM_ERR_ACCESS;

  if (GIFWriteImageInfo() != IM_ERR_NONE)
    return IM_ERR_ACCESS;

  /* initializes the hash table */
  memset(gif_data.HTable, 0xFF, GIF_HT_SIZE * sizeof(int));

  /* initializes compression data */

  imbyte byte_value = 8;
  imBinFileWrite(handle, &byte_value, 1, 1); /* Write the Code size to file. */

  gif_data.Buf[0] = 0;			  /* Nothing was output yet. */
  gif_data.BitsPerPixel = 8;
  gif_data.ClearCode = (1 << 8);
  gif_data.EOFCode = gif_data.ClearCode + 1;
  gif_data.RunningBits = 8 + 1;	 /* Number of bits per code. */
  gif_data.MaxCode1 = 1 << gif_data.RunningBits;	   /* Max. code + 1. */
  gif_data.CrntCode = GIF_FIRST_CODE;	   /* Signal that this is first one! */
  gif_data.CrntShiftState = 0;      /* No information in CrntShiftDWord. */
  gif_data.CrntShiftDWord = 0;

  gif_data.RunningCode = gif_data.EOFCode + 1;

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  gif_data.step = 0;          /* interlaced step */

  return iGIFCompressOutput(&gif_data, handle, gif_data.ClearCode);
}

int imFileFormatGIF::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading GIF...");

  int row = 0, error;
  for (int i = 0; i < this->height; i++)
  {
    error = iGIFDecompressLine(&gif_data, handle, (imbyte*)this->line_buffer, this->width);
    if (error != IM_ERR_NONE)
      return IM_ERR_ACCESS;

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;

	  if (gif_data.interlaced)
	  {
      row += InterlacedJumps[gif_data.step];

      if (row > this->height-1)
      {
        gif_data.step++;
        row = InterlacedOffset[gif_data.step];
      }
	  }
	  else
      row++;
  }

  /* Skip remaining empty blocks of the image data */
  if (iGIFSkipSubBlocks(handle) != IM_ERR_NONE)
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

int imFileFormatGIF::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing GIF...");

  int row = 0, error;
  for (int i = 0; i < this->height; i++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    error = iGIFCompressLine(&gif_data, handle, (imbyte*)this->line_buffer, this->width);

    if (error != IM_ERR_NONE)
      return IM_ERR_ACCESS;

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;

	  if (gif_data.interlaced)
	  {
      row += InterlacedJumps[gif_data.step];

      if (row > this->height-1)
      {
        gif_data.step++;
        row = InterlacedOffset[gif_data.step];
      }
	  }
	  else
      row++;
  }

  /* writes the end picture code */
  iGIFCompressOutput(&gif_data, handle, gif_data.CrntCode);
  iGIFCompressOutput(&gif_data, handle, gif_data.EOFCode);
  iGIFCompressOutput(&gif_data, handle, GIF_FLUSH_OUTPUT);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  this->image_count++;
  return IM_ERR_NONE;
}

int imFormatGIF::CanWrite(const char* compression, int color_mode, int data_type) const
{
  int color_space = imColorModeSpace(color_mode);

  if (color_space != IM_MAP && color_space != IM_GRAY && color_space != IM_BINARY)
    return IM_ERR_DATA;                       
                                              
  if (data_type != IM_BYTE)
    return IM_ERR_DATA;

  if (!compression || compression[0] == 0)
    return IM_ERR_NONE;

  if (!imStrEqual(compression, "LZW"))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}

