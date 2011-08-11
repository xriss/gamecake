/* GLUT Capture Sample

   Uses GLUT for user interface
        OpenGL for drawing
        IM for image I/O and capture

  Needs "opengl32.lib", "glu32.lib", "glut32.lib", "vfw32.lib", "strmiids.lib", 
        "im.lib", "im_capture.lib", "im_avi.lib" and "im_process.lib".

   Control Keys:

  <Esc> - Terminates
  <Space> - Activates/Deactivates the capturing.
  <F1>, <F2> , <F3>, etc - Shows capture configuration dialogs, in general 2, but can have more.
  <v> - Starts to save every frame in an AVI file.
  <b> - Process a background image using an average of N frames.
  <s> - Saves the background image in a BMP file.
  <1>, <2>, etc - Activates an processing operation. 
                  Only operation 1 is working, it subtracts the background image if one was created.
  <0> - Deactivates all the processing operations.

  ATENTION: These keys works at the GLUT window. 
            But the text input in done at the console window. 
            Check the correct window focus before typing keys.
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>

#include <GL/glut.h>
#include <im.h>
#include <im_capture.h>
#include <im_image.h>
#include <im_convert.h>
#include <im_process.h>
#include <im_format_avi.h>
#include <im_util.h>


/* Global Variables */
imVideoCapture* myVideoCap;    /* capture control */
imImage* image = NULL;         /* capture buffer  */
unsigned char* gl_data = NULL; /* opengl display buffer */

char video_filename[512] = "";     
imFile* video_file = NULL;

imImage* back_image = NULL;        /* background image */
imImage* back_acum = NULL;         /* aux image for background image calculation */
int back_count = 0;                /* number of images to average */
int back_index = 0;                /* average image counter */

int user_key[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

static void SimpleBackSub(imbyte *map, imbyte *back_map, int count, float tol)
{
  int i;
  for (i = 0; i < count; i++)
  {
    int diff = map[i] - back_map[i];
    if (diff < 0) diff = -diff;

    if(diff <= tol)
      map[i] = 0;
  }
}

static float tol = 10; /* you should use some key to change this */

void capture_process(int* user_key, imImage* image, imImage* back_image)
{
  if (user_key[0] && back_image) /* '1' */
  {
    int i;
    for (i = 0; i < image->depth; i++)  /* notice that here depth is always 3 */
    {
      SimpleBackSub((imbyte*)image->data[i], (imbyte*)back_image->data[i], image->count, tol);
    }
  }

  /***** call other operations here ******/
}


/* Aux to draw a number in the display */
void display_number(int num)
{
  int i;
  char msg[30];
  sprintf(msg,"%4d", num);
  glColor3f(1.0f,0.0f,0.0f); 
  glRasterPos2f(10.f,10.f); 
  for(i = 0; msg[i]; i++) 
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, msg[i]);
}

/* GLUT display callback */
/* called everytime the window needs to be updated */
void display(void) 
{
  if (!image)
    return;

  /* Draws the captured image at (0,0) */
  glRasterPos2f(0.f, 0.f); 
  glDrawPixels(image->width, image->height, GL_RGB, GL_UNSIGNED_BYTE, gl_data);

  glutSwapBuffers();
}


/* GLUT reshape callback */
/* called everytime the window changes its size */
void reshape(int w, int h)
{
  glViewport(0, 0, w, h);
} 


/* GLUT idle callback */
/* called when there is no events to be processed */
void idle(void)
{
  if (imVideoCaptureLive(myVideoCap, -1))
  {
    imVideoCaptureFrame(myVideoCap, image->data[0], IM_RGB, 1000);

    if (back_image && back_index < back_count)
    {
      /* calculating the background image */

      imProcessUnArithmeticOp(image, back_acum, IM_UN_INC);  /* back_image += image */
      back_index++;

      if (back_index == back_count)  /* last sum, divide by N */
      {
        imProcessArithmeticConstOp(back_acum, (float)back_count, back_image, IM_BIN_DIV);
        printf("Background image updated.\n");
      }
    }
    else
    {
      /* call some processing */
      capture_process(user_key, image, back_image);

      if (video_file)
      {
        imFileWriteImageInfo(video_file, image->width, image->height, IM_RGB, IM_BYTE);
        imFileWriteImageData(video_file, image->data[0]);
      }
    }

    imConvertPacking(image->data[0], gl_data, image->width, image->height, image->depth, image->depth, image->data_type, 0);
    display();
  }
}


/* OpenGL initialization */
void glinit(void)
{
  if (!image)
    return;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D (0.0, (GLdouble)image->width, 0.0, (GLdouble)image->height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}


/* updates the capture image size and display buffer size */
void updatebuffer(void)
{
  int width, height;

  /* retrieve the image size */
  imVideoCaptureGetImageSize(myVideoCap, &width, &height);

  if (width != image->width || height != image->height)
  {
    /* fix the buffer size */
    imImageReshape(image, width, height);
    gl_data = realloc(gl_data, image->size);

    /* fix the window size */
    glutReshapeWindow(image->width, image->height); 

    /* re-inititalizes the OpenGL */
    glinit();
  }
}


/* GLUT function key callback */
/* called everytime a function key is pressed */
void parsefunckey(int key, int x, int y)
{
  switch (key) {
    case GLUT_KEY_F1:   /* F1, F2, F.. = shows the capture configuration dialogs */
    case GLUT_KEY_F2:                                      
    case GLUT_KEY_F3:
    case GLUT_KEY_F4:
    case GLUT_KEY_F5:
    case GLUT_KEY_F6:
    case GLUT_KEY_F7:
    case GLUT_KEY_F8:
      imVideoCaptureLive(myVideoCap, 0);      /* deactivate the capture before calling the dialog */
      imVideoCaptureShowDialog(myVideoCap, key - GLUT_KEY_F1, NULL);
      updatebuffer();
      imVideoCaptureLive(myVideoCap, 1);
      break;
  }
}

/* GLUT key callback */
/* called everytime an ASCII key is pressed */
void parsekey(unsigned char key, int x, int y)
{
  int error, index;
  switch (key) {
    case 27:                                       /* Esc = terminates */
      printf("\nTerminating...\n");
      imVideoCaptureDisconnect(myVideoCap);
      imVideoCaptureDestroy(myVideoCap);
      imImageDestroy(image);
      if (video_file) 
      {
        imFileClose(video_file);
        printf("AVI file created.\n");
      }
      free(gl_data);
      exit(1);
    case ' ':                                      /* Space = activates/deactivates the capturing */
      if (imVideoCaptureLive(myVideoCap, -1))
        imVideoCaptureLive(myVideoCap, 0);
      else
        imVideoCaptureLive(myVideoCap, 1);
      break;
    case 'v':
      if (video_file) 
      {
        imFileClose(video_file);
        printf("AVI file created.\n");
        video_file = NULL;
        break;
      }
      printf("Enter the AVI file name:\n  >");
      scanf("%s", video_filename);
      video_file = imFileNew(video_filename, "AVI", &error);
      if (!video_file)
        printf("Error creating video file.\n");
      else
      {
        imFileSetInfo(video_file, "CUSTOM");  /* shows the compression options dialog */
        imFileWriteImageInfo(video_file, image->width, image->height, IM_RGB, IM_BYTE);
      }
      break;
    case 'b':
      if (back_image) 
      {
        imImageDestroy(back_image);
        imImageDestroy(back_acum);
      }
      printf("Enter the number of images to average:\n  >");
      scanf("%d", &back_count);
      back_acum = imImageCreate(image->width, image->height, IM_RGB, IM_USHORT);
      back_image = imImageClone(image);
      back_index = 0;
      break;
    case 's':
      if (back_image) 
      {
        char filename[512];
        imFile* ifile;
        printf("Enter the BMP file name:\n  >");
        scanf("%s", filename);
        ifile = imFileNew(filename, "BMP", &error);
        if (!ifile) {
          printf("Error creating image file.\n"); return;
        }
        imFileSaveImage(ifile, back_image);
        imFileClose(ifile);
        printf("BMP file created.\n");
      }
      break;
    case '0':
      memset(user_key, 0, 9*sizeof(int));
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      index = key - '1';
      user_key[index] = user_key[index]? 0: 1;  /* switch state */
      if (user_key[index])
        printf("Processing %c activated. \n", key);
      else
        printf("Processing %c deactivated. \n", key);
      return;
    default:
      glutPostRedisplay();
      return;
  }
}


/* Returns a capture device */
int getcapture(void)
{
  int i;
  int cap_count = imVideoCaptureDeviceCount();
  if (cap_count == 1)  /* only one device */
    return 0;

  printf("Enter the capture device number to use:\n");
  for (i = 0; i < cap_count; i++)
  {
    printf("  %s\n", imVideoCaptureDeviceDesc(i));
  }

  printf("  > ");
  scanf("%d", &i);
  if (i < 0 || i >= cap_count) 
    return 0;

  return i;
}


/* Initializes the capture device */
int initcapture(void)
{
  int width, height;

  /* creates an IM video capture manager */
  myVideoCap = imVideoCaptureCreate();
  if (!myVideoCap) {
    printf("No capture device found.\n"); return 0;
  }

  /* connects the device */
  if (!imVideoCaptureConnect(myVideoCap, getcapture())) {
    imVideoCaptureDestroy(myVideoCap);
    printf("Can not connect to capture device.\n");  return 0;
  }

  if (!imVideoCaptureLive(myVideoCap, 1)) {
    imVideoCaptureDisconnect(myVideoCap);
    imVideoCaptureDestroy(myVideoCap);
    printf("Can not activate capturing.\n");  return 0;
  }

  /* retrieve the image size */
  imVideoCaptureGetImageSize(myVideoCap, &width, &height);

  /* alocates the buffers */
  image = imImageCreate(width, height, IM_RGB, IM_BYTE);
  gl_data = malloc(image->size);

  return 1;
}


int main(int argc, char* argv[])
{
  printf("GLUT Capture\n");
  printf("  <Esc> - Terminates.\n"
         "  <Space> - Activates/Deactivates the capturing.\n"
         "  <F1>, <F2> , <F3>, ... - Shows capture configuration dialogs.\n"
         "  <v> - Starts to save every frame in an AVI file.\n"
         "  <b> - Process a background image using an average of N frames.\n"
         "  <s> - Saves the background image in a BMP file.\n"
         "  <1>, <2>, ... - Activates an processing operation.\n"
         "  <0> - Deactivates all the processing operations.\n\n");

  /* Initializes the capture device */
  if (!initcapture()) 
    return 1;

  imFormatRegisterAVI();

  /* GLUT initialization */
  glutInit(&argc, argv);                           
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);     
  glutInitWindowPosition(100, 100);                
  glutInitWindowSize(image->width, image->height); 
  glutCreateWindow("GLUT Capture");         

  glClearColor(0., 0., 0., 1.0);                   /* window background */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);           /* data alignment is 1 */

  /* register GLUT callbacks */
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(parsekey);
  glutSpecialFunc(parsefunckey);
  glutIdleFunc(idle);

  /* OpenGL initialization */
  glinit();

  /* GLUT message loop */
  glutMainLoop();

  return 0;
}
