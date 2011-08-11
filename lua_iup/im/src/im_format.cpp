/** \file
 * \brief File Format Access
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format.cpp,v 1.3 2009/11/23 17:13:05 scuri Exp $
 */


#include <stdlib.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#include "im.h"
#include "im_format.h"
#include "im_util.h"


static imFormat* iFormatList[50];
static int iFormatCount = 0;
static int iFormatRegistredAll = 0;

void imFormatRemoveAll(void)
{
  for (int i = 0; i < iFormatCount; i++)
  {
    imFormat* iformat = iFormatList[i];
    delete iformat;
    iFormatList[i] = NULL;
  }
  iFormatCount = 0;
  iFormatRegistredAll = 0;
}

void imFormatRegister(imFormat* iformat)
{
  iFormatList[iFormatCount] = iformat;
  iFormatCount++;
}

static imFormat* iFormatFind(const char* format)
{
  assert(format);

  if (!iFormatRegistredAll) 
  {
    imFormatRegisterInternal();
    iFormatRegistredAll = 1;
  }

  for (int i = 0; i < iFormatCount; i++)
  {
    imFormat* iformat = iFormatList[i];
    if (imStrEqual(format, iformat->format))
      return iformat;
  }
  return NULL;
}

void imFormatList(char** format_list, int *format_count)
{
  assert(format_list);
  assert(format_count);

  if (!iFormatRegistredAll) 
  {
    imFormatRegisterInternal();
    iFormatRegistredAll = 1;
  }

  static char format_list_buffer[50][50];

  *format_count = iFormatCount;
  for (int i = 0; i < iFormatCount; i++)
  {
    imFormat* iformat = iFormatList[i];
    strcpy(format_list_buffer[i], iformat->format);
    format_list[i] = format_list_buffer[i];
  }
}

int imFormatInfo(const char* format, char* desc, char* ext, int *can_sequence)
{
  imFormat* iformat = iFormatFind(format);
  if (!iformat) return IM_ERR_FORMAT;

  if (desc) strcpy(desc, iformat->desc);
  if (ext) strcpy(ext, iformat->ext);
  if (can_sequence) *can_sequence = iformat->can_sequence;

  return IM_ERR_NONE;
}

int imFormatCompressions(const char* format, char** comp, int *comp_count, int color_mode, int data_type)
{
  imFormat* iformat = iFormatFind(format);
  if (!iformat) return IM_ERR_FORMAT;

  int count = 0;

  static char comp_buffer[50][50];

  for (int i = 0; i < iformat->comp_count; i++)
  {
    if (color_mode == -1 || data_type == -1 || 
        iformat->CanWrite(iformat->comp[i], color_mode, data_type) == IM_ERR_NONE)
    {
      strcpy(comp_buffer[count], iformat->comp[i]);
      comp[count] = comp_buffer[count];
      count++;
    }
  }

  *comp_count = count;

  return IM_ERR_NONE;
}

int imFormatCanWriteImage(const char* format, const char* compression, int color_mode, int data_type)
{
  assert(format);

  imFormat* iformat = iFormatFind(format);
  if (!iformat) return IM_ERR_FORMAT;

  int error = iformat->CanWrite(compression, color_mode, data_type);
  return error;
}

static char* utlFileGetExt(const char *file_name)
{
  int len = strlen(file_name);

  // Starts at the last character
  int offset = len - 1;
  while (offset != 0)
  {
    // if found a path separator, no extension found
    if (file_name[offset] == '\\' || file_name[offset] == '/')
      return NULL;

    if (file_name[offset] == '.')
    {
      offset++;
      break;
    }

    offset--;
  }

  // if at the first character, no extension found
  if (offset == 0) 
    return NULL;

  int ext_size = len - offset + 1;
  char* file_ext = (char*)malloc(ext_size);

  for (int i = 0; i < ext_size-1; i++)
    file_ext[i] = (char)tolower(file_name[i+offset]);
  file_ext[ext_size-1] = 0;

  return file_ext;
}

imFileFormatBase* imFileFormatBaseOpen(const char* file_name, int *error)
{
  int i;

  assert(file_name);
  assert(error);

  if (!iFormatRegistredAll) 
  {
    imFormatRegisterInternal();
    iFormatRegistredAll = 1;
  }

  int* ext_mark = new int [iFormatCount];
  memset(ext_mark, 0, sizeof(int)*iFormatCount);

  // Search for the extension first, this usually is going to speed the search
  char* extension = utlFileGetExt(file_name);
  if (extension)
  {
    for(i = 0; i < iFormatCount; i++)
    {
      imFormat* iformat = iFormatList[i];

      if (strstr(iformat->ext, extension) != NULL)
      {
        ext_mark[i] = 1; // Mark this format to avoid testing it again in the next phase

        imFileFormatBase* ifileformat = iformat->Create();
        *error = ifileformat->Open(file_name);                                               
        if (*error != IM_ERR_NONE && *error != IM_ERR_FORMAT)  // Error situation that must abort
        {                                                      // Only IM_ERR_FORMAT is considered here
          free(extension);
          delete [] ext_mark;
          delete ifileformat;
          return NULL;
        }
        else if (*error == IM_ERR_NONE) // Sucessfully oppened the file
        {
          free(extension);
          delete [] ext_mark;
          return ifileformat;
        }
        else
        {
          /* Other errors, release the format and test another one */
          delete ifileformat;
        }
      }
    }

    free(extension);
  }

  // If the search did not work, try all the formats
  // except those already tested.

  for(i = 0; i < iFormatCount; i++)
  {
    if (!ext_mark[i])
    {
      imFormat* iformat = iFormatList[i];
      imFileFormatBase* ifileformat = iformat->Create();
      *error = ifileformat->Open(file_name);
      if (*error != IM_ERR_NONE && *error != IM_ERR_FORMAT)  // Error situation that must abort
      {                                                      // Only IM_ERR_FORMAT is a valid error here
        delete [] ext_mark;
        delete ifileformat;
        return NULL;
      }
      else if (*error == IM_ERR_NONE) // Sucessfully oppened the file
      {
        delete [] ext_mark;
        return ifileformat;
      }
      else
      {
        /* Other errors, release the format and test another one */
        delete ifileformat;
      }
    }
  }

  *error = IM_ERR_FORMAT;
  delete [] ext_mark;
  return NULL;
}

imFileFormatBase* imFileFormatBaseOpenAs(const char* file_name, const char* format, int *error)
{
  assert(file_name);
  assert(format);
  assert(error);

  if (!iFormatRegistredAll) 
  {
    imFormatRegisterInternal();
    iFormatRegistredAll = 1;
  }

  imFormat* iformat = iFormatFind(format);
  if (!format)
  {
    *error = IM_ERR_FORMAT;
    return NULL;
  }

  imFileFormatBase* ifileformat = iformat->Create();
  *error = ifileformat->Open(file_name);
  if (*error != IM_ERR_NONE && *error != IM_ERR_FORMAT)  // Error situation that must abort
  {
    delete ifileformat;
    return NULL;
  }
  else if (*error == IM_ERR_NONE) // Sucessfully oppened the file
    return ifileformat;
  else
  {
    *error = IM_ERR_FORMAT;
    delete ifileformat;
    return NULL;
  }
}

imFileFormatBase* imFileFormatBaseNew(const char* file_name, const char* format, int *error)
{
  assert(file_name);
  assert(format);
  assert(error);

  imFormat* iformat = iFormatFind(format);
  if (!iformat)
  {
    *error = IM_ERR_FORMAT;
    return NULL;
  }

  imFileFormatBase* ifileformat = iformat->Create();
  *error = ifileformat->New(file_name);
  if (*error)
  {
    delete ifileformat;
    return NULL;
  }

  return ifileformat;
}
