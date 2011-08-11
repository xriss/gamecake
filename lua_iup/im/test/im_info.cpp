/* IM 3 sample that returns information about a file.

  Needs "im.lib".

  Usage: im_info <file_name>

    Example: im_info test.tif
*/

#include <im.h>
#include <im_util.h>
#include <im_binfile.h>
#include <im_format_jp2.h>
#include <im_format_avi.h>
#include <im_format_wmv.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

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

int FindZero(imbyte* data, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (data[i] == 0)
      return 1;
  }
  return 0;
}

char* AttribData2Str(const void* data, int data_type)
{
  static char data_str[50] = "";

  switch(data_type)
  {
  case IM_BYTE:
    sprintf(data_str, "%3d", (int)(*((imbyte*)data)));
    break;
  case IM_USHORT:
    sprintf(data_str, "%5d", (int)(*((imushort*)data)));
    break;
  case IM_INT:  
    sprintf(data_str, "%5d", *((int*)data));
    break;
  case IM_FLOAT:
    sprintf(data_str, "%5.2f", (double)(*((float*)data)));
    break;
  case IM_CFLOAT:
    {
      float *c = (float*)data;
      sprintf(data_str, "%5.2g, %5.2f", (double)*c, (double)*(c+1));
    }
    break;
  }

  return data_str;
}

char* GetSizeDesc(double *size)
{
  char* size_desc;

  if (*size < 1024)
    size_desc = "b";
  else
  {
    *size /= 1024;

    if (*size < 1024)
      size_desc = "Kb";
    else
    {
      *size /= 1024;
      size_desc = "Mb";
    }
  }

  return size_desc;
}

unsigned long FileSize(const char* file_name)
{
  imBinFile* bfile = imBinFileOpen(file_name);
  if (!bfile) return 0;

  unsigned long file_size = imBinFileSize(bfile);

  imBinFileClose(bfile);
  return file_size;
}

void PrintImageInfo(const char* file_name)
{
  printf("IM Info\n");
  printf("  File Name:\n    %s\n", file_name);

  int error;
  imFile* ifile = imFileOpen(file_name, &error);
  if (!ifile) 
  {
    PrintError(error);
    return;
  }

  double file_size = FileSize(file_name);
  printf("  File Size: %.2f %s\n", file_size, GetSizeDesc(&file_size));

  char format[10];
  char compression[20];
  int image_count;
  imFileGetInfo(ifile, format, compression, &image_count);

  char format_desc[50];
  imFormatInfo(format, format_desc, NULL, NULL);
  printf("  Format: %s - %s\n", format, format_desc);
  printf("  Compression: %s\n", compression);
  printf("  Image Count: %d\n", image_count);
  
  for (int i = 0; i < image_count; i++)
  {
    int width, height, color_mode, data_type;

    error = imFileReadImageInfo(ifile, i, &width, &height, &color_mode, &data_type);
    if (error != IM_ERR_NONE)
    {
      PrintError(error);
      imFileClose(ifile);  
      return;
    }

    printf("  Image #%d\n", i);
    printf("    Width: %d\n", width);
    printf("    Height: %d\n", height);
    printf("    Color Space: %s\n", imColorModeSpaceName(color_mode));
    printf("      Has Alpha: %s\n", imColorModeHasAlpha(color_mode)? "Yes": "No");
    printf("      Is Packed: %s\n", imColorModeIsPacked(color_mode)? "Yes": "No");
    printf("      Is Top Down: %s\n", imColorModeIsTopDown(color_mode)? "Yes": "No");
    printf("    Data Type: %s\n", imDataTypeName(data_type));

    double image_size = imImageDataSize(width, height, color_mode, data_type);
    printf("    Data Size: %.2f %s\n", image_size, GetSizeDesc(&image_size));

    char* attrib_list[50];  // should be dynamic allocated
    int attrib_list_count;
    imFileGetAttributeList(ifile, attrib_list, &attrib_list_count);

    for (int a = 0; a < attrib_list_count; a++)
    {
      if (a == 0)
        printf("    Attributes:\n");

      int attrib_data_type, attrib_count;
      const void* attrib_data = imFileGetAttribute(ifile, attrib_list[a], &attrib_data_type, &attrib_count);

      if (attrib_count == 1)
        printf("      %s: %s\n", attrib_list[a], AttribData2Str(attrib_data, attrib_data_type));
      else if (attrib_data_type == IM_BYTE && FindZero((imbyte*)attrib_data, attrib_count))
        printf("      %s: %s\n", attrib_list[a], attrib_data);
      else
        printf("      %s: %s %s ...\n", attrib_list[a], AttribData2Str(attrib_data, attrib_data_type), AttribData2Str((imbyte*)attrib_data + imDataTypeSize(attrib_data_type), attrib_data_type));
    }
  }
    
  imFileClose(ifile);  
}

int main(int argc, char* argv[])
{
//  imFormatRegisterJP2();
//  imFormatRegisterAVI();
//  imFormatRegisterWMV();   

  if (argc < 2)
  {
    printf("Invalid number of arguments.\n");
    return 0;
  }

  PrintImageInfo(argv[1]);

  return 1;
}
