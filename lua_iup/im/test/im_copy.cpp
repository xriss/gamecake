/* IM 3 sample that copies an image from one file to another. 
   It is good to test the file formats read and write.
   If the destiny does not supports the input image it aborts and returns an error.

  Needs "im.lib".

  Usage: im_copy <input_file_name> <output_file_name> [<output_format> [<output_compression]]

    Example: im_copy test.tif test_proc.tif
*/

#include <im.h>
#include <im_util.h>
#include <im_format_avi.h>
#include <im_format_wmv.h>

#include <stdio.h>
#include <stdlib.h>


void PrintError(int error)
{
  switch (error)
  {
  case IM_ERR_OPEN:
    printf("Error Opening File.\n");
    break;
  case IM_ERR_MEM:
    printf("Insuficient memory.\n");
    break;
  case IM_ERR_ACCESS:
    printf("Error Accessing File.\n");
    break;
  case IM_ERR_DATA:
    printf("Image type not Suported.\n");
    break;
  case IM_ERR_FORMAT:
    printf("Invalid Format.\n");
    break;
  case IM_ERR_COMPRESS:
    printf("Invalid or unsupported compression.\n");
    break;
  default:
    printf("Unknown Error.\n");
  }
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Invalid number of arguments.\n");
    return 0;
  }

//  imFormatRegisterAVI();
//  imFormatRegisterWMV();

  void* data = NULL;
  imFile* ifile = NULL;
  imFile* ofile = NULL;

  int error;
  ifile = imFileOpen(argv[1], &error);
  if (!ifile) 
    goto man_error;

  char format[10];
  char compression[20];
  int image_count;
  imFileGetInfo(ifile, format, compression, &image_count);

  ofile = imFileNew(argv[2], (argc < 3)? format: argv[3], &error);
  if (!ofile)
    goto man_error;

  if (argc < 4)
    imFileSetInfo(ofile, compression);
  else
    imFileSetInfo(ofile, argv[4]);

  for (int i = 0; i < image_count; i++)
  {
    int size, max_size = 0;
    int width, height, color_mode, data_type;
    error = imFileReadImageInfo(ifile, i, &width, &height, &color_mode, &data_type);
    if (error != IM_ERR_NONE)
      goto man_error;

    size = imImageDataSize(width, height, color_mode, data_type);

    if (size > max_size)
    {
      data = realloc(data, size);
      max_size = size;
    }

    error = imFileReadImageData(ifile, data, 0, -1);
    if (error != IM_ERR_NONE)
      goto man_error;
    
    char* attrib_list[50];
    int attrib_list_count;
    imFileGetAttributeList(ifile, attrib_list, &attrib_list_count);

    for (int a = 0; a < attrib_list_count; a++)
    {
      int attrib_data_type, attrib_count;
      const void* attrib_data = imFileGetAttribute(ifile, attrib_list[a], &attrib_data_type, &attrib_count);
      imFileSetAttribute(ofile, attrib_list[a], attrib_data_type, attrib_count, attrib_data);
    }

    if (imColorModeSpace(color_mode) == IM_MAP)
    {
      long palette[256];
      int palette_count;
      imFileGetPalette(ifile, palette, &palette_count);
      imFileSetPalette(ifile, palette, palette_count);
    }

    error = imFileWriteImageInfo(ofile, width, height, color_mode, data_type);
    if (error != IM_ERR_NONE)
      goto man_error;

    error = imFileWriteImageData(ofile, data);
    if (error != IM_ERR_NONE)
      goto man_error;

    printf(".");
  }
  printf("done");

  free(data);
  imFileClose(ifile);  
  imFileClose(ofile);  

  return 1;

man_error:
  PrintError(error);
  if (data) free(data);
  if (ifile) imFileClose(ifile);
  if (ofile) imFileClose(ofile);
  return 0;
}
