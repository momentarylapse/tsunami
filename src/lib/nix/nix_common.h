/*----------------------------------------------------------------------------*\
| Nix common                                                                   |
| -> common data references                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#pragma once
#if HAS_LIB_GL

#include <stdio.h>


#ifdef OS_WINDOWS
	#include <windows.h>
	#include <GL\glew.h>
	#include <gl\gl.h>
	//#include <gl\glext.h>
	//#include <gl\wglext.h>
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
#if HAS_LIB_GLEW
	#include <GL/glew.h>
#else
	#define GL_GLEXT_PROTOTYPES
#endif
	#include <GL/gl.h>
	#include <GL/glext.h>
#endif


namespace nix {

extern int current_program;


extern mat4 view_matrix, projection_matrix;
extern mat4 model_matrix, model_view_projection_matrix;



void TestGLError(const char *);

struct Fog {
	color _color;
	float density;
};
extern Fog fog;


};


#endif
