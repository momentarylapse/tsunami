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

#ifndef _NIX_EXISTS_
#define _NIX_EXISTS_


#include "../config.h"


#include "nix_config.h"
#include "nix_draw.h"
#include "nix_light.h"
#include "nix_vertexbuffer.h"
#include "nix_shader.h"
#include "nix_view.h"
#include "nix_textures.h"

namespace nix{

extern string version;


//--------------------------------------------------------------------//
//                     all about graphics                             //
//--------------------------------------------------------------------//
void avi_close(int texture);

void _cdecl Init();
void KillDeviceObjects();
void ReincarnateDeviceObjects();
void Kill();

// engine properties
void _cdecl SetWire(bool enabled);
void _cdecl SetCull(int mode);
void _cdecl SetZ(bool write,bool test);
void _cdecl SetAlpha(int mode);
void _cdecl SetAlpha(int src,int dst);
void _cdecl SetAlpha(float factor);
void _cdecl SetAlphaM(int mode);
void _cdecl SetAlphaSD(int src,int dst);
void _cdecl SetFog(int mode, float start, float end, float density, const color &c);
void _cdecl EnableFog(bool enabled);
void _cdecl SetStencil(int mode,unsigned long param=0);
void _cdecl SetOffset(float offset);


extern rect target_rect;

};

#endif

#endif
