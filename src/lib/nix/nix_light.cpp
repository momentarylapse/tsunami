/*----------------------------------------------------------------------------*\
| Nix light                                                                    |
| -> handling light sources                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix{



Material material;


void set_material(const color &albedo, float roughness, float metal, const color &emission) {
	material.albedo = albedo;
	material.roughness = roughness;
	material.metal = metal;
	//material.specular = (1 - roughness);
	//material.shininess = 5 / (1.1f - roughness);//shininess;
	material.emission = emission;
}



};

#endif
