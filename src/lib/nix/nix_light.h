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

void _cdecl EnableLighting(bool enabled);
void _cdecl SetLightRadial(int index, const vector &pos, float radius, const color &col, float harshness);
void _cdecl SetLightDirectional(int index, const vector &dir, const color &col, float harshness);
void _cdecl EnableLight(int index, bool enabled);
void _cdecl SetMaterial(const color &ambient, const color &diffuse, const color &specular, float shininess, const color &emission);



struct Material
{
	color ambient;
	color diffusive;
	color specular;
	color emission;
	float shininess;
};
extern Material material;

struct Light
{
	bool enabled;
	color col;
	float harshness;
	vector pos;
	float radius;
};
extern Light lights[8];

};

#endif

#endif
