/*----------------------------------------------------------------------------*\
| Nix textures                                                                 |
| -> texture loading and handling                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.11.02 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_TEXTURES_EXISTS_
#define _NIX_TEXTURES_EXISTS_

namespace nix{

// textures
void init_textures();
void ReleaseTextures();
void ReincarnateTextures();
void ProgressTextureLifes();



class Texture
{
public:
	enum class Type {
		DEFAULT,
		DYNAMIC,
		CUBE,
		DEPTH,
		IMAGE
	};
	Type type;
	string filename;
	int width, height;
	bool valid;
	int life_time;
	
	unsigned int texture;
	unsigned int frame_buffer;
	unsigned int depth_render_buffer;
	unsigned int internal_format;

	Texture();
	~Texture();
	void _cdecl __init__();
	void _cdecl __delete__();

	void _cdecl overwrite(const Image &image);
	void _cdecl read(Image &image);
	void _cdecl read_float(Array<float> &data);
	void _cdecl reload();
	void _cdecl unload();
	//void _cdecl set_video_frame(int frame);
	//void _cdecl video_move(float elapsed);
};

class DynamicTexture : public Texture
{
public:
	DynamicTexture(int width, int height);
	void _cdecl __init__(int width, int height);
};

class ImageTexture : public Texture
{
public:
	ImageTexture(int width, int height, const string &format);
	void _cdecl __init__(int width, int height, const string &format);
};

class DepthTexture : public Texture
{
public:
	DepthTexture(int width, int height);
	void _cdecl __init__(int width, int height);
};

class CubeMap : public Texture
{
public:
	CubeMap(int size);
	void _cdecl __init__(int size);

	void _cdecl overwrite_side(int side, const Image &image);
	void _cdecl fill_side(int side, Texture *source);
	void _cdecl render_to_cube_map(vector &pos, callback_function *render_scene, int mask);
};


Texture* _cdecl LoadTexture(const string &filename);
void _cdecl SetTexture(Texture *texture);
void _cdecl SetTextures(Array<Texture*> &textures);

extern Array<Texture*> textures;
extern int tex_cube_level;

};

#endif
