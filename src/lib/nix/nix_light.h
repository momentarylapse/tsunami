/*----------------------------------------------------------------------------*\
| Nix light                                                                    |
| -> handling light sources                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#pragma once


#include "../math/matrix.h"
#include "../math/vector.h"
#include "../image/color.h"


namespace nix{

void _cdecl set_material(const color &albedo, float roughness, float metal, const color &emission);



struct Material {
	color albedo;
	color emission;
	float roughness, metal;
};
extern Material material;

// compatible with default_shader_3d (binding=1)
struct BasicLight {
	alignas(16) matrix proj;
	alignas(16) vector pos;
	alignas(16) vector dir;
	alignas(16) color col;
	alignas(16) float radius;
	float theta, harshness;
};

};

#endif
