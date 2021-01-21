/*----------------------------------------------------------------------------*\
| Nix shader                                                                   |
| -> shader files                                                              |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_SHADER_EXISTS_
#define _NIX_SHADER_EXISTS_

#include "../base/pointer.h"

namespace nix{

class UniformBuffer {
public:
	unsigned int buffer;
	UniformBuffer();
	~UniformBuffer();

	void __init__();
	void __delete__();
	void update(void *data, int size);
	void update_array(const DynamicArray &a);
};

void BindUniform(UniformBuffer *ub, int binding);

class Shader : public Sharable<Empty> {
public:
	Path filename;
	int program;
	Shader();
	~Shader();
	void _cdecl set_float(int location, float f);
	void _cdecl set_int(int location, int i);
	void _cdecl set_data(int location, const float *data, int size);
	void _cdecl set_matrix(int location, const matrix &m);
	void _cdecl set_color(int location, const color &c);
	int _cdecl get_location(const string &name);
	bool _cdecl link_uniform_block(const string &name, int binding);
	
	void _cdecl dispatch(int nx, int ny, int nz);

	void find_locations();
	void set_default_data();

	enum {
		LOCATION_MATRIX_MVP,
		LOCATION_MATRIX_M,
		LOCATION_MATRIX_V,
		LOCATION_MATRIX_P,
		LOCATION_TEX,
		LOCATION_TEX_CUBE = LOCATION_TEX + NIX_MAX_TEXTURELEVELS,
		LOCATION_MATERIAL_ALBEDO,
		LOCATION_MATERIAL_ROUGHNESS,
		LOCATION_MATERIAL_METAL,
		LOCATION_MATERIAL_EMISSION,
		NUM_LOCATIONS
	};

	int location[NUM_LOCATIONS];


	static Shader* _cdecl load(const Path &filename);
	static Shader* _cdecl create(const string &source);
	void _cdecl update(const string &source);


	static Shader *default_2d;
	static Shader *default_3d;
	static Shader *default_load;
	static Shader *_current_;
};


void init_shaders();
void _cdecl DeleteAllShaders();
void _cdecl SetShader(Shader *s);

extern Path shader_dir;


};

#endif

#endif
