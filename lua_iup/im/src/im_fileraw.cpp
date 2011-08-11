/** \file
 * \brief RAW File Format Open/New Functions
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_fileraw.cpp,v 1.3 2009/09/10 17:33:35 scuri Exp $
 */

#include "im.h"
#include "im_image.h"
#include "im_util.h"
#include "im_counter.h"
#include "im_raw.h"
#include "im_format.h"
#include "im_format_raw.h"

#include <stdlib.h>
#include <assert.h>


imFile* imFileOpenRaw(const char* file_name, int *error)
{
  assert(file_name);

  imFormat* iformat = imFormatInitRAW();
  imFileFormatBase* ifileformat = iformat->Create();
  *error = ifileformat->Open(file_name);
  if (*error)
  {
    delete ifileformat;
    return NULL;
  }

  imFileClear(ifileformat);

  ifileformat->attrib_table = new imAttribTable(599);

  ifileformat->counter = imCounterBegin(file_name);

  return ifileformat;
}

imFile* imFileNewRaw(const char* file_name, int *error)
{
  assert(file_name);

  imFormat* iformat = imFormatInitRAW();
  imFileFormatBase* ifileformat = iformat->Create();
  *error = ifileformat->New(file_name);
  if (*error) 
  {
    delete ifileformat;
    return NULL;
  }
   
  imFileClear(ifileformat);

  ifileformat->is_new = 1;
  ifileformat->image_count = 0;
  ifileformat->compression[0] = 0;

  ifileformat->attrib_table = new imAttribTable(101);

  ifileformat->counter = imCounterBegin(file_name);

  return ifileformat;
}
