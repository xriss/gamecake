/** \file
 * \brief System Dependent Binary File Access (UNIX)
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_sysfile_unix.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "im_util.h"
#include "im_binfile.h"


class imBinSystemFile: public imBinFileBase
{
protected:
  int FileHandle, 
      Error;

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
  int mode = O_RDONLY;
#ifdef O_BINARY
    mode |= O_BINARY;
#endif        
  this->FileHandle = open(pFileName, mode, 0);
  if (this->FileHandle < 0) 
    this->Error = errno;
  else
    this->Error = 0;
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 0;
}

void imBinSystemFile::New(const char* pFileName)
{
  int mode = O_WRONLY | O_CREAT | O_TRUNC;           
#ifdef O_BINARY
    mode |= O_BINARY;
#endif        
  this->FileHandle = open(pFileName, mode, 0666); // User/Group/Other can read and write
  if (this->FileHandle < 0) 
    this->Error = errno;
  else
    this->Error = 0;
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 1;
}

void imBinSystemFile::Close()
{
  assert(this->FileHandle > -1);
  int ret = close(this->FileHandle);
  if (ret < 0)
    this->Error = errno;
  else
    this->Error = 0;
}

int imBinSystemFile::HasError() const
{
  if (this->FileHandle < 0 || this->Error) return 1;
  return 0;
}

unsigned long imBinSystemFile::ReadBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle > -1);
	int ret = read(this->FileHandle, pValues, (size_t)pSize);
  if (ret < 0)
    this->Error = errno;
  else
    this->Error = 0;
  return ret < 0? 0: ret;
}
                             
unsigned long imBinSystemFile::WriteBuf(void* pValues, unsigned long pSize)
{
  assert(this->FileHandle > -1);
  int ret = write(this->FileHandle, pValues, (size_t)pSize);
  if (ret < 0)
    this->Error = errno;
  else
    this->Error = 0;
  return ret < 0? 0: ret;
}

void imBinSystemFile::SeekTo(unsigned long pOffset)
{
  assert(this->FileHandle > -1);
  int ret = lseek(this->FileHandle, pOffset, SEEK_SET);
  if (ret < 0)
    this->Error = errno;
  else
    this->Error = 0;
}

void imBinSystemFile::SeekOffset(long pOffset)
{
  assert(this->FileHandle > -1);
  int ret = lseek(this->FileHandle, pOffset, SEEK_CUR);
  if (ret < 0)
    this->Error = errno;
  else
    this->Error = 0;
}

void imBinSystemFile::SeekFrom(long pOffset)
{
  assert(this->FileHandle > -1);
  int ret = lseek(this->FileHandle, pOffset, SEEK_END);
  if (ret < 0)
    this->Error = errno;
  else
    this->Error = 0;
}

unsigned long imBinSystemFile::Tell() const
{
  assert(this->FileHandle > -1);
  long offset = lseek(this->FileHandle, 0L, SEEK_CUR);
  return offset < 0? 0: offset;
}

unsigned long imBinSystemFile::FileSize()
{
  assert(this->FileHandle > -1);
  long lCurrentPosition = lseek(this->FileHandle, 0L, SEEK_CUR);
  long lSize = lseek(this->FileHandle, 0L, SEEK_END);
  lseek(this->FileHandle, lCurrentPosition, SEEK_SET);
  return lSize < 0? 0: lSize;
}

int imBinSystemFile::EndOfFile() const
{
  assert(this->FileHandle > -1);
  long lCurrentPosition = lseek(this->FileHandle, 0L, SEEK_CUR);
  long lSize = lseek(this->FileHandle, 0L, SEEK_END);
  lseek(this->FileHandle, lCurrentPosition, SEEK_SET);
  return lCurrentPosition == lSize? 1: 0;
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
  // the file was successfully opened already by the client

  int *s = (int*)pFileName;
  this->FileHandle = s[0];
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 0;
  this->Error = 0;
}

void imBinSystemFileHandle::New(const char* pFileName)
{
  // the file was successfully opened already the client

  int *s = (int*)pFileName;
  this->FileHandle = s[0];
  InitByteOrder(imBinCPUByteOrder());
  this->IsNew = 1;
  this->Error = 0;
}

void imBinSystemFileHandle::Close()
{
  // does nothing, the client must close the file
}
