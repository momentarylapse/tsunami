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


// TODO: better names
void SetMaterial(const color &diffuse, float ambient, float specular, float shininess, const color &emission) {
	material.ambient = ambient;
	material.diffusive = diffuse;
	material.specular = specular;
	material.shininess = shininess;
	material.emission = emission;
}



};

#endif
