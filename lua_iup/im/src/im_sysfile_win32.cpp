/** \file
 * \brief System Dependent Binary File Access.
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_sysfile_win32.cpp,v 1.2 2009/10/01 16:12:24 scuri Exp $
 */

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#include "im_util.h"
#include "im_binfile.h"

/* not defined in VC6 */
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

class imBinSystemFile: public imBinFileBase
{
protected:
  HANDLE FileHandle;
  int Error;

  unsigned long ReadBuf(void* pValues, unsigned long pSize);
  unsigned long WriteBuf(void* pValues, unsigned long pSize);

public:
  virtual void Open(const char* pFileName);
  virtual void New(const char* pFileName);
  virtual void Close();

  unsigned long FileSize();
  int HasError() const;
  void SeekTo(unsigned long pOffset);
  void SeekOffset(long pOffset);
  void SeekFrom(long pOffset);
  unsigned long Tell() const;
  int EndOfFile() const;
};

imBinFileBase* iBinSystemFileNewFunc()
{
  return new imBinSystemFile();
}

void imBinSystemFile::Open(const char* pFileName)
{
  this->FileHandle = CreateFile(pFileName, GENERIC_READ, 
                                           FILE_SHARE_READ, 
                                           NULL, 
                                           OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);
  this->Error = (this->FileHandle == INVALID_HANDLE_VALUE)? 1: 0;
  SetLastError(NO_ERROR);
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 0;
}

void imBinSystemFile::New(const char* pFileName)
{
  this->FileHandle = CreateFile(pFileName, GENERIC_READ | GENERIC_WRITE, 
                                           0, 
                                           NULL, 
                                           CREATE_ALWAYS,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);
  this->Error = (this->FileHandle == INVALID_HANDLE_VALUE)? 1: 0;
  SetLastError(NO_ERROR);
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 1;
}

void imBinSystemFile::Close()
{
  if (this->FileHandle != INVALID_HANDLE_VALUE) 
    CloseHandle(this->FileHandle);

  this->FileHandle = INVALID_HANDLE_VALUE;
  this->Error = 1;
}

unsigned long imBinSystemFile::FileSize()
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  this->Error = 0;
  DWORD Size = GetFileSize(this->FileHandle, NULL);
  if (Size == INVALID_FILE_SIZE)
    this->Error = 1;
  return Size;
}

unsigned long imBinSystemFile::ReadBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  this->Error = 0;
  DWORD dwSize = 0;
  ReadFile(this->FileHandle, pValues, pSize, &dwSize, NULL);
  if (dwSize != pSize)
    this->Error = 1;
  return dwSize;
}
                             
unsigned long imBinSystemFile::WriteBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  this->Error = 0;
  DWORD dwSize = 0;
  WriteFile(this->FileHandle, pValues, pSize, &dwSize, NULL);
  if (dwSize != pSize)
    this->Error = 1;
  return dwSize;
}

int imBinSystemFile::HasError() const
{
  return this->Error;
}
        
void imBinSystemFile::SeekTo(unsigned long pOffset)
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  this->Error = 0;
  DWORD ret = SetFilePointer(this->FileHandle, pOffset, NULL, FILE_BEGIN);
  if (ret == INVALID_SET_FILE_POINTER)
    this->Error = 1;
}

void imBinSystemFile::SeekOffset(long pOffset)
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  this->Error = 0;
  DWORD ret = SetFilePointer(this->FileHandle, pOffset, NULL, FILE_CURRENT);
  if (ret == INVALID_SET_FILE_POINTER)
    this->Error = 1;
}

void imBinSystemFile::SeekFrom(long pOffset)
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  this->Error = 0;
  DWORD ret = SetFilePointer(this->FileHandle, pOffset, NULL, FILE_END);
  if (ret == INVALID_SET_FILE_POINTER)
    this->Error = 1;
}

unsigned long imBinSystemFile::Tell() const
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  return SetFilePointer(this->FileHandle, 0, NULL, FILE_CURRENT);
}

int imBinSystemFile::EndOfFile() const
{
  assert(this->FileHandle != INVALID_HANDLE_VALUE);
  DWORD cur_pos = SetFilePointer(this->FileHandle, 0, NULL, FILE_CURRENT);
  DWORD end_pos = SetFilePointer(this->FileHandle, 0, NULL, FILE_END);
  SetFilePointer(this->FileHandle, cur_pos, NULL, FILE_CURRENT);
  return (cur_pos == end_pos)? 1: 0;
}



class imBinSystemFileHandle: public imBinSystemFile
{
public:
  virtual void Open(const char* pFileName);
  virtual void New(const char* pFileName);
  virtual void Close();
};

imBinFileBase* iBinSystemFileHandleNewFunc()
{
  return new imBinSystemFileHandle();
}

void imBinSystemFileHandle::Open(const char* pFileName)
{
  // the file was successfully opened already the client 

  HANDLE file_handle = (HANDLE)pFileName;
  this->FileHandle = file_handle;
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 0;
  this->Error = 0;
}

void imBinSystemFileHandle::New(const char* pFileName)
{
  // the file was successfully opened already the client 

  HANDLE file_handle = (HANDLE)pFileName;
  this->FileHandle = file_handle;
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 1;
  this->Error = 0;
}

void imBinSystemFileHandle::Close()
{
  // does nothing, the client must close the file
}
