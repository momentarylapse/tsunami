/*----------------------------------------------------------------------------*\
| Nix                                                                          |
| -> abstraction layer for api (graphics)                                      |
| -> OpenGL and DirectX9 supported                                             |
|   -> able to switch in runtime                                               |
| -> mathematical types and functions                                          |
|   -> vector, matrix, matrix3, quaternion, plane, color                       |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.10.27 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#pragma once


#include "../config.h"

#include "nix_config.h"
#include "nix_draw.h"
#include "nix_light.h"
#include "nix_vertexbuffer.h"
#include "nix_shader.h"
#include "nix_view.h"
#include "nix_textures.h"
#include "nix_framebuffer.h"
#include "nix_buffer.h"

namespace nix {

extern string version;

enum class CullMode;
enum class AlphaMode;
enum class Alpha;
enum class StencilOp;

//--------------------------------------------------------------------//
//                     all about graphics                             //
//--------------------------------------------------------------------//
void avi_close(int texture);

void _cdecl init();
void kill();
void flush();

enum class FogMode;

// engine properties
void _cdecl set_wire(bool enabled);
void _cdecl set_cull(CullMode mode);
void _cdecl set_z(bool write, bool test);
void _cdecl _set_alpha(AlphaMode mode);
void _cdecl set_alpha(Alpha src, Alpha dst);
void _cdecl _set_alpha_mode(AlphaMode mode);
void _cdecl set_alpha_sd(Alpha src, Alpha dst);
void _cdecl disable_alpha();
void _cdecl set_fog(FogMode mode, float start, float end, float density, const color &c);
void _cdecl enable_fog(bool enabled);
void _cdecl set_stencil(StencilOp op, unsigned long param=0);
void _cdecl set_offset(float offset);


extern rect target_rect;

};

#endif
