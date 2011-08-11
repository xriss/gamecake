/** \file
 * \brief Binary File Access
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_binfile.cpp,v 1.3 2010/01/26 19:13:02 scuri Exp $
 */


#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>

#include "im_util.h"
#include "im_binfile.h"


/**************************************************
                imBinMemoryFile
***************************************************/

class imBinMemoryFile: public imBinFileBase
{
protected:
  unsigned long CurrentSize, BufferSize;  
  unsigned char* Buffer, *CurPos;
  int Error;
  float Reallocate;
  imBinMemoryFileName* file_name;

  unsigned long ReadBuf(void* pValues, unsigned long pSize);
  unsigned long WriteBuf(void* pValues, unsigned long pSize);

public:
  void Open(const char* pFileName);
  void New(const char* pFileName);
  void Close() {} // Does nothing, the memory belongs to the user

  unsigned long FileSize();
  int HasError() const;
  void SeekTo(unsigned long pOffset);
  void SeekOffset(long pOffset);
  void SeekFrom(long pOffset);
  unsigned long Tell() const;
  int EndOfFile() const;
};

static imBinFileBase* iBinMemoryFileNewFunc()
{
  return new imBinMemoryFile();
}

void imBinMemoryRelease(unsigned char *buffer)
{
  free(buffer);
}

void imBinMemoryFile::Open(const char* pFileName)
{
  this->file_name = (imBinMemoryFileName*)pFileName;

  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 0;

  assert(this->file_name->size);

  this->Buffer = this->file_name->buffer;
  this->BufferSize = this->file_name->size;
  this->Reallocate = this->file_name->reallocate;
  this->CurrentSize = this->BufferSize;
  this->CurPos = this->Buffer;
  this->Error = 0;
}

void imBinMemoryFile::New(const char* pFileName)
{
  this->file_name = (imBinMemoryFileName*)pFileName;

  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 1;

  assert(this->file_name->size);

  this->Buffer = this->file_name->buffer;
  this->BufferSize = this->file_name->size;
  this->Reallocate = this->file_name->reallocate;
  this->CurrentSize = 0;

  if (!this->Buffer)
  {
    this->Buffer = (unsigned char*)malloc(this->BufferSize);
    this->file_name->buffer = this->Buffer;
  }

  this->CurPos = this->Buffer;
  this->Error = 0;
}

unsigned long imBinMemoryFile::ReadBuf(void* pValues, unsigned long pSize)
{
  assert(this->Buffer);

  unsigned long lOffset = this->CurPos - this->Buffer;

  this->Error = 0;
  if (lOffset + pSize > this->CurrentSize)
  {
    this->Error = 1;
    pSize = this->CurrentSize - lOffset;
  }

  if (pSize)
  {
    memcpy(pValues, this->CurPos, pSize);
    this->CurPos += pSize;
  }

  return pSize;
}
                             
unsigned long imBinMemoryFile::WriteBuf(void* pValues, unsigned long pSize)
{
  assert(this->Buffer);

  unsigned long lOffset = this->CurPos - this->Buffer;

  this->Error = 0;
  if (lOffset + pSize > this->BufferSize)
  {
    if (this->Reallocate != 0.0)
    {
      unsigned long nSize = this->BufferSize;
      while (lOffset + pSize > nSize)
        nSize += (unsigned long)(this->Reallocate*(float)this->BufferSize);

      this->Buffer = (unsigned char*)realloc(this->Buffer, nSize);

      if (this->Buffer)
      {
        this->BufferSize = nSize;
        this->file_name->buffer = this->Buffer;
        this->file_name->size = this->BufferSize;
      }
      else
      {
        this->Buffer = this->file_name->buffer;
        this->Error = 1;
        pSize = this->BufferSize - lOffset;
      }
      
      this->CurPos = this->Buffer + lOffset;
    }
    else
    {
      this->Error = 1;
      pSize = this->BufferSize - lOffset;
    }
  }

  memcpy(this->CurPos, pValues, pSize);

  if (lOffset + pSize > this->CurrentSize)
    this->CurrentSize = lOffset + pSize;

  this->CurPos += pSize;

  return pSize;
}

unsigned long imBinMemoryFile::FileSize()
{
  assert(this->Buffer);
  return this->CurrentSize;
}

int imBinMemoryFile::HasError() const
{
  if (!this->Buffer) return 1;
  return this->Error;
}

void imBinMemoryFile::SeekTo(unsigned long pOffset)
{
  assert(this->Buffer);

  this->Error = 0;
  if (pOffset > this->BufferSize)
  {
    this->Error = 1;
    return;
  }

  this->CurPos = this->Buffer + pOffset;

  /* update size if we seek after EOF */
  if (pOffset > this->CurrentSize)
    this->CurrentSize = pOffset;
}

void imBinMemoryFile::SeekFrom(long pOffset)
{
  assert(this->Buffer);

  /* remember that offset is usually a negative value in this case */

  this->Error = 0;
  if (this->CurrentSize + pOffset > this->BufferSize || 
      (long)this->CurrentSize + pOffset < 0)
  {
    this->Error = 1;
    return;
  }

  this->CurPos = this->Buffer + this->CurrentSize + pOffset;

  /* update size if we seek after EOF */
  if (pOffset > 0)
    this->CurrentSize = this->CurrentSize + pOffset;
}

void imBinMemoryFile::SeekOffset(long pOffset)
{
  assert(this->Buffer);
  long lOffset = this->CurPos - this->Buffer;

  this->Error = 0;
  if (lOffset + pOffset < 0 || lOffset + pOffset > (long)this->BufferSize)
  {
    this->Error = 1;
    return;
  }

  this->CurPos += pOffset;

  /* update size if we seek after EOF */
  if (lOffset + pOffset > (long)this->CurrentSize)
    this->CurrentSize = lOffset + pOffset;
}

unsigned long imBinMemoryFile::Tell() const
{
  assert(this->Buffer);
  unsigned long lOffset = this->CurPos - this->Buffer;
  return lOffset;
}

int imBinMemoryFile::EndOfFile() const
{
  assert(this->Buffer);
  unsigned long lOffset = this->CurPos - this->Buffer;
  return lOffset == this->CurrentSize? 1: 0;
}

/**************************************************
                imBinSubFile
**************************************************/

static imBinFileBase* iBinFileBaseHandle(const char* pFileName);

class imBinSubFile: public imBinFileBase
{
protected:
  imBinFileBase* FileHandle;
  unsigned long StartOffset;

  unsigned long ReadBuf(void* pValues, unsigned long pSize);
  unsigned long WriteBuf(void* pValues, unsigned long pSize);

public:
  void Open(const char* pFileName);
  void New(const char* pFileName);
  void Close() {} // Does nothing, the file should be close by the parent file.

  unsigned long FileSize();
  int HasError() const;
  void SeekTo(unsigned long pOffset);
  void SeekOffset(long pOffset);
  void SeekFrom(long pOffset);
  unsigned long Tell() const;
  int EndOfFile() const;
};

static imBinFileBase* iBinSubFileNewFunc()
{
  return new imBinSubFile();
}

void imBinSubFile::Open(const char* pFileName)
{
  this->FileHandle = iBinFileBaseHandle(pFileName);
  this->FileByteOrder = this->FileByteOrder;
  this->IsNew = 0;
  
  StartOffset = this->FileHandle->Tell();
}

void imBinSubFile::New(const char* pFileName)
{
  this->FileHandle = iBinFileBaseHandle(pFileName);
  this->FileByteOrder = this->FileByteOrder;
  this->IsNew = 1;
  
  StartOffset = this->FileHandle->Tell();
}

unsigned long imBinSubFile::FileSize()
{
  assert(this->FileHandle);
  return this->FileHandle->FileSize();
}

unsigned long imBinSubFile::ReadBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle);
  return this->FileHandle->ReadBuf(pValues, pSize);
}
                             
unsigned long imBinSubFile::WriteBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle);
  return this->FileHandle->WriteBuf(pValues, pSize);
}

int imBinSubFile::HasError() const
{
  assert(this->FileHandle);
  return this->FileHandle->HasError();
}

void imBinSubFile::SeekTo(unsigned long pOffset)
{
  assert(this->FileHandle);
  this->FileHandle->SeekTo(StartOffset + pOffset);
}

void imBinSubFile::SeekOffset(long pOffset)
{
  assert(this->FileHandle);
  this->FileHandle->SeekOffset(pOffset);
}

void imBinSubFile::SeekFrom(long pOffset)
{
  assert(this->FileHandle);
  this->FileHandle->SeekFrom(pOffset);
}

unsigned long imBinSubFile::Tell() const
{
  assert(this->FileHandle);
  return this->FileHandle->Tell() - StartOffset;
}

int imBinSubFile::EndOfFile() const
{
  assert(this->FileHandle);
  return this->FileHandle->EndOfFile();
}

/**************************************************
                imBinStreamFile
**************************************************/

class imBinStreamFile: public imBinFileBase
{
protected:
  FILE* FileHandle;

  unsigned long ReadBuf(void* pValues, unsigned long pSize);
  unsigned long WriteBuf(void* pValues, unsigned long pSize);

public:
  void Open(const char* pFileName);
  void New(const char* pFileName);
  void Close();

  unsigned long FileSize();
  int HasError() const;
  void SeekTo(unsigned long pOffset);
  void SeekOffset(long pOffset);
  void SeekFrom(long pOffset);
  unsigned long Tell() const;
  int EndOfFile() const;
};

static imBinFileBase* iBinStreamFileNewFunc()
{
  return new imBinStreamFile();
}

void imBinStreamFile::Open(const char* pFileName)
{
  this->FileHandle = fopen(pFileName, "rb");
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 0;
}

void imBinStreamFile::New(const char* pFileName)
{
  this->FileHandle = fopen(pFileName, "wb");
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 1;
}

void imBinStreamFile::Close()
{
  if (this->FileHandle) fclose(this->FileHandle);
}

unsigned long imBinStreamFile::FileSize()
{
  assert(this->FileHandle);
  unsigned long lCurrentPosition = ftell(this->FileHandle);
  fseek(this->FileHandle, 0L, SEEK_END);
  unsigned long lSize = ftell(this->FileHandle);
  fseek(this->FileHandle, lCurrentPosition, SEEK_SET);
  return lSize;
}

unsigned long imBinStreamFile::ReadBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle);
	return fread(pValues, 1, pSize, this->FileHandle);
}
                             
unsigned long imBinStreamFile::WriteBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle);
	return fwrite(pValues, 1, pSize, this->FileHandle);
}

int imBinStreamFile::HasError() const
{
  if (!this->FileHandle) return 1;
  return ferror(this->FileHandle) == 0? 0: 1;
}

void imBinStreamFile::SeekTo(unsigned long pOffset)
{
  assert(this->FileHandle);
  fseek(this->FileHandle, pOffset, SEEK_SET);
}

void imBinStreamFile::SeekOffset(long pOffset)
{
  assert(this->FileHandle);
  fseek(this->FileHandle, pOffset, SEEK_CUR);
}

void imBinStreamFile::SeekFrom(long pOffset)
{
  assert(this->FileHandle);
  fseek(this->FileHandle, pOffset, SEEK_END);
}

unsigned long imBinStreamFile::Tell() const
{
  assert(this->FileHandle);
  return ftell(this->FileHandle);
}

int imBinStreamFile::EndOfFile() const
{
  assert(this->FileHandle);
  return feof(this->FileHandle) == 0? 0: 1;
}

/**************************************************
                 NewFuncModules
**************************************************/

/* implemented in "im_sysfile*.cpp" */
imBinFileBase* iBinSystemFileNewFunc();
imBinFileBase* iBinSystemFileHandleNewFunc();

#define MAX_MODULES 10

static imBinFileNewFunc iBinFileModule[MAX_MODULES] = 
{
  iBinSystemFileNewFunc,
  iBinStreamFileNewFunc, 
  iBinMemoryFileNewFunc,
  iBinSubFileNewFunc,
  iBinSystemFileHandleNewFunc
};
static int iBinFileModuleCount = 5;
static int iBinFileModuleCurrent = 0; // default module is the first

int imBinFileSetCurrentModule(int pModule)
{
  int old_module = iBinFileModuleCurrent;

  if (pModule >= iBinFileModuleCount)
    return -1;

  iBinFileModuleCurrent = pModule;

  return old_module;
}

extern "C" int imBinFileRegisterModule(imBinFileNewFunc pNewFunc)
{
  if (iBinFileModuleCount == MAX_MODULES) return -1;
  int id = iBinFileModuleCount;
  iBinFileModule[id] = pNewFunc;
  iBinFileModuleCount++;
  return id;
}

/**************************************************
                 imBinFile
**************************************************/

struct _imBinFile
{
  imBinFileBase* binfile;
};

imBinFile* imBinFileOpen(const char* pFileName)
{
  assert(pFileName);

  assert(iBinFileModuleCurrent < iBinFileModuleCount);
  assert(iBinFileModuleCurrent < MAX_MODULES);

  imBinFileNewFunc NewFunc = iBinFileModule[iBinFileModuleCurrent];
  imBinFileBase* binfile = NewFunc();

  binfile->Open(pFileName);
  if (binfile->HasError())
  {
    delete binfile;
    return NULL;
  }

  imBinFile* bfile = new imBinFile;
  bfile->binfile = binfile;

  return bfile;
}

imBinFile* imBinFileNew(const char* pFileName)
{
  assert(pFileName);

  imBinFileNewFunc NewFunc = iBinFileModule[iBinFileModuleCurrent];
  imBinFileBase* binfile = NewFunc();

  binfile->New(pFileName);
  if (binfile->HasError())
  {
    delete binfile;
    return NULL;
  }

  imBinFile* bfile = new imBinFile;
  bfile->binfile = binfile;

  return bfile;
}

void imBinFileClose(imBinFile* bfile)
{
  assert(bfile);
  bfile->binfile->Close();
  delete bfile->binfile;
  delete bfile;
}

int imBinFileByteOrder(imBinFile* bfile, int pByteOrder)
{
  assert(bfile);
  return bfile->binfile->InitByteOrder(pByteOrder);
}

int imBinFileError(imBinFile* bfile)
{
  assert(bfile);
  return bfile->binfile->HasError();
}

unsigned long imBinFileSize(imBinFile* bfile)
{
  assert(bfile);
  return bfile->binfile->FileSize();
}

unsigned long imBinFileRead(imBinFile* bfile, void* pValues, unsigned long pCount, int pSizeOf)
{
  assert(bfile);
  return bfile->binfile->Read(pValues, pCount, pSizeOf);
}

unsigned long imBinFileWrite(imBinFile* bfile, void* pValues, unsigned long pCount, int pSizeOf)
{
  assert(bfile);
  return bfile->binfile->Write(pValues, pCount, pSizeOf);
}

void imBinFileSeekTo(imBinFile* bfile, unsigned long pOffset)
{
  assert(bfile);
  bfile->binfile->SeekTo(pOffset);
}

void imBinFileSeekOffset(imBinFile* bfile, long pOffset)
{
  assert(bfile);
  bfile->binfile->SeekOffset(pOffset);
}

void imBinFileSeekFrom(imBinFile* bfile, long pOffset)
{
  assert(bfile);
  bfile->binfile->SeekFrom(pOffset);
}

unsigned long imBinFileTell(imBinFile* bfile)
{
  assert(bfile);
  return bfile->binfile->Tell();
}

int imBinFileEndOfFile(imBinFile* bfile)
{
  assert(bfile);
  return bfile->binfile->EndOfFile();
}

unsigned long imBinFilePrintf(imBinFile* bfile, char *format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  char buffer[4096];
  int size = vsprintf(buffer, format, arglist);
  return imBinFileWrite(bfile, buffer, size, 1);
}

int imBinFileReadInteger(imBinFile* handle, int *value)
{
  int i = 0, found = 0;
  char buffer[11], c;

  while (!found)
  {
    imBinFileRead(handle, &c, 1, 1);

    /* if it's an integer, increments the number of characters read */
    if ((c >= '0' && c <= '9') || (c == '-'))
    {
      buffer[i] = c;
      i++;
    }
    else
    {
      /* if it's not, and we read some characters, convert them to an integer */
      if (i > 0)
      {
        buffer[i] = 0;
        *value = atoi(buffer);
        found = 1;
      }
    }

    if (imBinFileError(handle) || i > 10)
      return 0;
  } 

  return 1;
}

int imBinFileReadFloat(imBinFile* handle, float *value)
{
  int i = 0, found = 0;
  char buffer[17], c;

  while (!found)
  {
    imBinFileRead(handle, &c, 1, 1);

    /* if it's a floating point number, increments the number of characters read */
    if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E')
    {
      buffer[i] = c;
      i++;
    }
    else
    {
      /* if it's not, and we read some characters convert them to an integer */
      if (i > 0)
      {
        buffer[i] = 0;
        *value = (float)atof(buffer);
        found = 1;
      }
    }

    if (imBinFileError(handle) || i > 16)
      return 0;
  } 

  return 1;
}

static imBinFileBase* iBinFileBaseHandle(const char* pFileName)
{
  imBinFile* bfile = (imBinFile*)pFileName;
  return (imBinFileBase*)bfile->binfile;
}
