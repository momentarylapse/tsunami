/*----------------------------------------------------------------------------*\
| Nix config                                                                   |
| -> configuration for nix                                                     |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2007.11.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if HAS_LIB_GL

#ifndef _NIX_CONFIG_EXISTS_
#define _NIX_CONFIG_EXISTS_

#include "../config.h"



// which developing environment?

#ifdef _MSC_VER
	#define NIX_IDE_VCS
	#if _MSC_VER >= 1400
		#define NIX_IDE_VCS8
	#else
		#define NIX_IDE_VCS6
	#endif
#else
	#define NIX_IDE_DEVCPP
#endif
//#define NIX_IDE_KDEVELOP ...?



// which graphics api possible?

#define NIX_API_NONE					0
#define NIX_API_OPENGL					2




#ifndef OS_WINDOWS
	#undef NIX_ALLOW_VIDEO_TEXTURE
#endif


#include <math.h>
#include "../base/base.h"
#include "../file/file.h"
//#include "../hui/hui.h"
#include "../math/math.h"


typedef void callback_function();


#define NIX_MAX_TEXTURELEVELS	8


// alpha modes
enum {
	ALPHA_NONE,
	ALPHA_COLOR_KEY = 10,
	ALPHA_COLOR_KEY_SMOOTH,
	ALPHA_COLOR_KEY_HARD,
	ALPHA_ADD,
	ALPHA_MATERIAL,
};

// alpha parameters ("functions")
enum {
	ALPHA_ZERO,
	ALPHA_ONE,
	ALPHA_SOURCE_COLOR,
	ALPHA_SOURCE_INV_COLOR,
	ALPHA_SOURCE_ALPHA,
	ALPHA_SOURCE_INV_ALPHA,
	ALPHA_DEST_COLOR,
	ALPHA_DEST_INV_COLOR,
	ALPHA_DEST_ALPHA,
	ALPHA_DEST_INV_ALPHA,
};

enum {
	CULL_NONE,
	CULL_CCW,
	CULL_CW,
	CULL_DEFAULT = CULL_CCW
};

enum {
	STENCIL_NONE,
	STENCIL_INCREASE,
	STENCIL_DECREASE,
	STENCIL_DECREASE_NOT_NEGATIVE,
	STENCIL_SET,
	STENCIL_MASK_EQUAL,
	STENCIL_MASK_NOT_EQUAL,
	STENCIL_MASK_LESS,
	STENCIL_MASK_LESS_EQUAL,
	STENCIL_MASK_GREATER,
	STENCIL_MASK_GREATER_EQUAL,
	STENCIL_RESET
};

enum {
	FOG_LINEAR,
	FOG_EXP,
	FOG_EXP2
};


namespace nix {

//extern int device_width, device_height;						// render target size (window, won't change)
extern int target_width, target_height;						// current render target size (window/texture)
extern bool Fullscreen;

extern Path texture_dir;
extern int MaxVideoTextureSize;

class VertexBuffer;
extern VertexBuffer *vb_temp; // vertex buffer for 1-frame geometries
};

#endif

#endif
