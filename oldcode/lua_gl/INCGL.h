

#if defined(ANDROID) 

#include "GLES/gl.h"
#include "GLES/glext.h"

#elif  defined(NACL)

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

//#include "EGL/egl.h"
//#include "EGL/eglext.h"

#else

#include "GL/gl.h"
#include "GL/glu.h"

#endif



