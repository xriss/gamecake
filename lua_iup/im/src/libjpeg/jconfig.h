/*
 * see jconfig.doc
 */

#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
#define HAVE_STDDEF_H
#define HAVE_STDLIB_H


#define HAVE_JFIO

#include "im_binfile.h"

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) imBinFileRead((imBinFile*)file, (buf), (sizeofbuf), 1))

#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) imBinFileWrite((imBinFile*)file, (buf), (sizeofbuf), 1))

#define  JFFLUSH(file) \
  ((void)(file))

#define JFERROR(file) \
  imBinFileError((imBinFile*)file)
