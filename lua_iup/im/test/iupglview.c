
#ifdef WIN32            
#include <windows.h>    /* necessary because of the Microsoft OpenGL headers dependency */
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <iup.h>
#include <iupgl.h>

#include <im.h>
#include <im_image.h>
#include <im_util.h>
#include <im_convert.h>

int app_repaint_cb(Ihandle* self)
{
  unsigned char* gl_data = (unsigned char*)IupGetAttribute(self, "APP_GL_DATA");
  int width = IupGetInt(self, "APP_GL_WIDTH");
  int height = IupGetInt(self, "APP_GL_HEIGHT");
  IupGLMakeCurrent(self);        /* activates this GL Canvas as the current drawing area. */
  glClear(GL_COLOR_BUFFER_BIT);  /* clears the back buffer */

  if (gl_data)
  {
    /* Draws the captured image at (0,0) */
    glRasterPos2f(0.f, 0.f); 
    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, gl_data);
  }

  IupGLSwapBuffers(self);        /* swap data from back buffer to front buffer */
  return IUP_DEFAULT;
} 

void appGLInit(int width, int height)
{
  glClearColor(0., 0., 0., 1.0);                   /* window background */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);           /* data alignment is 1 */

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D (0.0, (GLdouble)width, 0.0, (GLdouble)height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

int app_resize_cb(Ihandle* self, int width, int height)
{
  IupGLMakeCurrent(self);
  appGLInit(width, height);
  return IUP_DEFAULT;
} 

/* OpenGL does not supports palette based images, so convert to RGB */
/* this function can also be use for RGBA images */
void ConvertMapToGLData(unsigned char* data, int count, int depth, long* palette, int palette_count)
{
  int c, i;
  unsigned char r[256], g[256], b[256];

  unsigned char* src_data = data + count-1;
  unsigned char* dst_data = data + depth*(count-1);

  for (c = 0; c < palette_count; c++)
    imColorDecode(&r[c], &g[c], &b[c], palette[c]);

  for (i = 0; i < count; i++)
  {
    int index = *src_data;
    *dst_data       =  r[index];
    *(dst_data+1) = g[index];
    *(dst_data+2) = b[index];

    dst_data -= depth;
    src_data--;
  }
}

int app_open_cb(Ihandle* self)
{
  imFile* ifile;             /* file input */
  int ret, error;
  unsigned char* gl_data = (unsigned char*)IupGetAttribute(self, "APP_GL_DATA");
  char filename[1024] = ".\\*.*";

  /* get a file name */
  ret = IupGetFile(filename);
  if (ret == -1)
    return IUP_DEFAULT;

  ifile = imFileOpen(filename, &error);
  if (!ifile)
  {
    IupMessage("Error", "Error reading image file.");
    return IUP_DEFAULT;
  }

  {
    int width = 0, height = 0, file_color_mode, color_space;
    Ihandle* dialog = IupGetDialog(self);
    imFileReadImageInfo(ifile, 0, &width, &height, &file_color_mode, NULL);

    /* alocates the buffers */
    if (gl_data) free(gl_data);
    gl_data = malloc(width*height*3);
    IupSetAttribute(dialog, "APP_GL_DATA", gl_data);
    IupSetfAttribute(dialog, "APP_GL_WIDTH", "%d", width);
    IupSetfAttribute(dialog, "APP_GL_HEIGHT", "%d", height);

    imFileReadImageData(ifile, gl_data, 1, IM_PACKED);

    color_space = imColorModeSpace(file_color_mode);
    if (color_space == IM_MAP || color_space == IM_GRAY || color_space == IM_BINARY)
    {
      long palette[256];
      int palette_count;
      imFileGetPalette(ifile, palette, &palette_count);
      ConvertMapToGLData(gl_data, width*height, 3, palette, palette_count);
    }
  }

  imFileClose(ifile);

  return IUP_DEFAULT;
}

int app_exit_cb(Ihandle *self)
{
  unsigned char* gl_data = (unsigned char*)IupGetAttribute(self, "APP_GL_DATA");

  /* destroy buffers */
  if (gl_data) 
    free(gl_data);

  return IUP_CLOSE;
}

int app_about_cb(Ihandle *self)
{
  IupMessagef("About", "IUPGLView 1.0\n"
                       "Tecgraf/PUC-Rio\n"
                       " ---------------- \n"
                       "IUP Version %s\n"
                       "IM Version %s\n"
                       " ---------------- \n"
                       "OpenGL:\n"
                       "  Vendor: %s\n"
                       "  Renderer: %s\n"
                       "  Version: %s\n"
                       , IUP_RELEASE_VERSION, IM_VERSION, 
                         glGetString(GL_VENDOR),glGetString(GL_RENDERER),glGetString(GL_VERSION));
  return IUP_DEFAULT;
}

void mainMenuCreate(void) 
{
  Ihandle* file_menu = IupMenu(
     IupItem( "Open...", "app_open_cb"),
     IupSeparator(),
     IupItem( "Exit", "app_exit_cb"),
     NULL
  );

  Ihandle* menu = IupMenu(
     IupSubmenu("File", file_menu),
     NULL
  );

  /* this will be used by the dialog */
  IupSetHandle("app_menu", menu);

  IupSetFunction("app_open_cb", (Icallback)app_open_cb);
  IupSetFunction("app_exit_cb", (Icallback)app_exit_cb);
}

void mainDialogCreate(void)
{
  Ihandle *dialog, *box, *canvas;

  /* initialize interface */

  /* canvas for the image */

  canvas = IupGLCanvas("app_repaint_cb");
  IupSetAttribute(canvas, "BORDER", "NO");
  IupSetAttribute(canvas, "BUFFER", "DOUBLE");   /* use double buffer */
  IupSetAttribute(canvas, "RESIZE_CB", "app_resize_cb");   /* configure the resize callback */

  IupSetFunction("app_resize_cb", (Icallback)app_resize_cb);
  IupSetFunction("app_repaint_cb", (Icallback)app_repaint_cb);

  /* this is the most external box that puts together
     the toolbar, the two canvas and the status bar */
  box = IupSetAttributes(IupHbox(
            canvas, 
            NULL), "MARGIN=10x10");

  /* create the dialog and set its attributes */

  mainMenuCreate();

  dialog = IupDialog(box);
  IupSetAttribute(dialog, "MENU", "app_menu");     /* configure the menu */
  IupSetAttribute(dialog, "CLOSE_CB", "app_exit_cb");
  IupSetAttribute(dialog, "TITLE", "IUPGLView");
  IupSetAttribute(dialog, "RASTERSIZE", "680x380"); /* initial size */
  IupSetAttribute(dialog, "SHRINK", "YES");
  IupSetHandle("app_dialog", dialog);

  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
}

int main(int argc, char* argv[]) 
{
  /* IUP initialization */
  IupOpen();
  IupGLCanvasOpen();

  /* Create and show the main dialog */
  mainDialogCreate();

  /* IUP event loop */
  IupMainLoop();

  /* IUP closing */
  IupClose();

  return 0;
}
