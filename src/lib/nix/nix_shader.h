/*----------------------------------------------------------------------------*\
| Nix shader                                                                   |
| -> shader files                                                              |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_SHADER_EXISTS_
#define _NIX_SHADER_EXISTS_


namespace nix{

class Shader
{
public:
	string filename;
	int program;
	int reference_count;
	Shader();
	~Shader();
	void _cdecl unref();
	void _cdecl set_float(int location, float f);
	void _cdecl set_int(int location, int i);
	void _cdecl set_data(int location, const float *data, int size);
	void _cdecl set_matrix(int location, const matrix &m);
	void _cdecl set_color(int location, const color &c);
	void _cdecl get_data(const string &var_name, void *data, int size);
	int _cdecl get_location(const string &var_name);
	
	void _cdecl dispatch(int nx, int ny, int nz);

	void find_locations();
	void set_default_data();

	enum{
		LOCATION_MATRIX_MVP,
		LOCATION_MATRIX_M,
		LOCATION_MATRIX_V,
		LOCATION_MATRIX_P,
		LOCATION_MATRIX_P2D,
		LOCATION_TEX,
		LOCATION_TEX_CUBE = LOCATION_TEX + NIX_MAX_TEXTURELEVELS,
		LOCATION_CAM_POS,
		LOCATION_CAM_DIR,
		LOCATION_MATERIAL_AMBIENT,
		LOCATION_MATERIAL_DIFFUSIVE,
		LOCATION_MATERIAL_SPECULAR,
		LOCATION_MATERIAL_SHININESS,
		LOCATION_MATERIAL_EMISSION,
		LOCATION_LIGHT_COLOR,
		LOCATION_LIGHT_AMBIENT,
		LOCATION_LIGHT_SPECULAR,
		LOCATION_LIGHT_POS,
		LOCATION_LIGHT_RADIUS,
		LOCATION_FOG_COLOR,
		LOCATION_FOG_DENSITY,
		NUM_LOCATIONS
	};

	int location[NUM_LOCATIONS];
};


void init_shaders();
void _cdecl DeleteAllShaders();
Shader* _cdecl LoadShader(const string &filename);
Shader* _cdecl CreateShader(const string &source);
void _cdecl SetShader(Shader *s);
void _cdecl SetOverrideShader(Shader *s);

extern Shader *default_shader_2d;
extern Shader *default_shader_3d;

extern string shader_dir;

};

#endif
