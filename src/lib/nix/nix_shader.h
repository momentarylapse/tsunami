/*----------------------------------------------------------------------------*\
| Nix shader                                                                   |
| -> shader files                                                              |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_SHADER_EXISTS_
#define _NIX_SHADER_EXISTS_


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

class Shader {
public:
	Path filename;
	int program;
	int reference_count;
	Shader();
	~Shader();
	void _cdecl unref();
	Shader _cdecl *ref();
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
		LOCATION_MATERIAL_AMBIENT,
		LOCATION_MATERIAL_DIFFUSIVE,
		LOCATION_MATERIAL_SPECULAR,
		LOCATION_MATERIAL_SHININESS,
		LOCATION_MATERIAL_EMISSION,
		NUM_LOCATIONS
	};

	int location[NUM_LOCATIONS];


	static Shader* _cdecl load(const Path &filename);
	static Shader* _cdecl create(const string &source);
};


void init_shaders();
void _cdecl DeleteAllShaders();
void _cdecl SetShader(Shader *s);
void _cdecl SetOverrideShader(Shader *s);

extern Shader *default_shader_2d;
extern Shader *default_shader_3d;

extern Path shader_dir;


};

#endif

#endif
