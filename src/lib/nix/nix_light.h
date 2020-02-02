/*----------------------------------------------------------------------------*\
| Nix light                                                                    |
| -> handling light sources                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_LIGHT_EXISTS_
#define _NIX_LIGHT_EXISTS_


namespace nix{

void _cdecl EnableLighting(bool enabled);
void _cdecl SetLightRadial(int index, const vector &pos, float radius, const color &diffuse, float ambient, float specular);
void _cdecl SetLightDirectional(int index, const vector &dir, const color &diffuse, float ambient, float specular);
void _cdecl EnableLight(int index, bool enabled);
void _cdecl SetAmbientLight(const color &c);
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
	color diffusive;
	float ambient, specular;
	vector pos;
	float radius;
};
extern Light lights[8];

};

#endif
