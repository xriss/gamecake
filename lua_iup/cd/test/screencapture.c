#include <stdio.h>
#include <stdlib.h>
#include <cd.h>
#include <cdnative.h>
#include <im.h>

void main()
{
  cdCanvas *cd_canvas;
  unsigned char *red, *green, *blue;
  int width, height, size;

  cd_canvas = cdCreateCanvas(CD_NATIVEWINDOW, NULL);
  if (!cd_canvas)
  {
    printf("Error creating canvas.\n");
    return;
  }        
  
  cdActivate(cd_canvas);

  cdGetCanvasSize(&width, &height, NULL, NULL);
  size = width * height;
  red = (unsigned char*)calloc(size, 1);
  green = (unsigned char*)calloc(size, 1);
  blue = (unsigned char*)calloc(size, 1);

  cdGetImageRGB(red, green, blue, 0, 0, width, height);
  imSaveRGB(width, height, IM_JPG|IM_COMPRESSED, red, green, blue, "scap.jpg");

  cdKillCanvas(cd_canvas);
  
  free(red);
  free(green);
  free(blue);
}
