/*----------------------------------------------------------------------------*\
| Nix config                                                                   |
| -> configuration for nix                                                     |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2007.11.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if HAS_LIB_GL

#pragma once

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

struct vec3;
struct rect;
struct mat4;
struct color;


typedef void callback_function();


#define NIX_MAX_TEXTURELEVELS	8

namespace nix {

// alpha modes
enum class AlphaMode {
	NONE,
	COLOR_KEY = 10,
	COLOR_KEY_SMOOTH,
	COLOR_KEY_HARD,
	ADD,
	MATERIAL,
};

// alpha parameters ("functions")
enum class Alpha {
	ZERO,
	ONE,
	SOURCE_COLOR,
	SOURCE_INV_COLOR,
	SOURCE_ALPHA,
	SOURCE_INV_ALPHA,
	DEST_COLOR,
	DEST_INV_COLOR,
	DEST_ALPHA,
	DEST_INV_ALPHA,
};

enum class CullMode {
	NONE,
	BACK,
	FRONT
};

enum class Orientation {
	CW,
	CCW
};

enum class StencilOp {
	NONE,
	INCREASE,
	DECREASE,
	DECREASE_NOT_NEGATIVE,
	SET,
	MASK_EQUAL,
	MASK_NOT_EQUAL,
	MASK_LESS,
	MASK_LESS_EQUAL,
	MASK_GREATER,
	MASK_GREATER_EQUAL,
	RESET
};

enum class FogMode {
	LINEAR,
	EXP,
	EXP2
};

enum class PrimitiveTopology {
	TRIANGLES = 0x0004, //GL_TRIANGLES,
	TRIANGLE_FAN = 0x0006, //GL_TRIANGLE_FAN;
	LINES = 0x0001, //GL_LINES,
	LINE_STRIP = 0x0003, //GL_LINE_STRIP,
	POINTS = 0x0000, //GL_POINTS,
	PATCHES = 0x000e, //GL_PATCHES
};


//extern int device_width, device_height;						// render target size (window, won't change)
extern int target_width, target_height;						// current render target size (window/texture)

};

#endif

