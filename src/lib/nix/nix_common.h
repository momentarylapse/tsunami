/*----------------------------------------------------------------------------*\
| Nix common                                                                   |
| -> common data references                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if HAS_LIB_GL

#include <stdio.h>

#ifdef OS_WINDOWS
	#define _WIN32_WINDOWS 0x500
	#include <stdio.h>
	#include <io.h>
	#include <direct.h>
	#include <mmsystem.h>
	#pragma warning(disable : 4995)
	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		#include "vfw.h"
	#endif
#endif

#ifdef OS_LINUX
#ifdef NIX_ALLOW_FULLSCREEN
	#include <X11/extensions/xf86vmode.h>
#endif
	#include <X11/keysym.h>
	#include <stdlib.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/stat.h>
#endif

#ifdef OS_WINDOWS
	#include <gl\gl.h>
	#include <gl\glext.h>
	#include <gl\wglext.h>
#endif
#ifdef OS_LINUX
	#define GL_GLEXT_PROTOTYPES
	#include <GL/glx.h>
	#include <GL/gl.h>
	#include <GL/glext.h>
#endif


namespace nix {

extern int current_program;


extern matrix view_matrix, projection_matrix;
extern matrix world_matrix, world_view_projection_matrix;



void TestGLError(const char *);

struct Fog {
	color _color;
	float density;
};
extern Fog fog;


};


#endif
