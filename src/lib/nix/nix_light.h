/*----------------------------------------------------------------------------*\
| Nix light                                                                    |
| -> handling light sources                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_LIGHT_EXISTS_
#define _NIX_LIGHT_EXISTS_


namespace nix{

void _cdecl SetMaterial(const color &ambient, const color &diffuse, const color &specular, float shininess, const color &emission);



struct Material {
	color ambient;
	color diffusive;
	color specular;
	color emission;
	float shininess;
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

#endif
