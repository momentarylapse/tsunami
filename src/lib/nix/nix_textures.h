/*----------------------------------------------------------------------------*\
| Nix textures                                                                 |
| -> texture loading and handling                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.11.02 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if HAS_LIB_GL

#ifndef _NIX_TEXTURES_EXISTS_
#define _NIX_TEXTURES_EXISTS_

#include "../base/pointer.h"
#include "../file/path.h"

namespace nix{

// textures
void init_textures();
void release_textures();
void reincarnate_textures();



class Texture : public Sharable<Empty> {
public:
	enum class Type {
		DEFAULT,
		DYNAMIC,
		CUBE,
		DEPTH,
		IMAGE,
		VOLUME,
		MULTISAMPLE,
		RENDERBUFFER
	};
	Type type;
	Path filename;
	int width, height, nz, samples;
	bool valid;
	
	unsigned int texture;
	unsigned int internal_format;

	Texture();
	Texture(int width, int height, const string &format);
	Texture(int nx, int ny, int nz, const string &format);
	~Texture();
	void _cdecl __init__(int width, int height, const string &format);
	void _cdecl __init3__(int nx, int ny, int nz, const string &format);
	void _cdecl __delete__();

	void _cdecl overwrite(const Image &image);
	void _cdecl read(Image &image);
	void _cdecl read_float(Array<float> &data);
	void _cdecl write_float(Array<float> &data, int nx, int ny, int nz);
	void _cdecl reload();
	void _cdecl unload();

	void _cdecl set_options(const string &op) const;

	static Texture* _cdecl load(const Path &filename);
};


class TextureMultiSample : public Texture {
public:
	TextureMultiSample(int width, int height, int samples, const string &format);
	void _cdecl __init__(int width, int height, int samples, const string &format);
};

class ImageTexture : public Texture {
public:
	ImageTexture(int width, int height, const string &format);
	void _cdecl __init__(int width, int height, const string &format);
};

class DepthBuffer : public Texture {
public:
	DepthBuffer(int width, int height);
	void _cdecl __init__(int width, int height);
};

class RenderBuffer : public Texture {
public:
	RenderBuffer(int width, int height);
	RenderBuffer(int width, int height, int samples);
};

class CubeMap : public Texture {
public:
	CubeMap(int size);
	void _cdecl __init__(int size);

	void _cdecl overwrite_side(int side, const Image &image);
	void _cdecl fill_side(int side, Texture *source);
	void _cdecl render_to_cube_map(vector &pos, callback_function *render_scene, int mask);
};


void _cdecl set_texture(Texture *texture);
void _cdecl set_textures(const Array<Texture*> &textures);

//extern Array<Texture*> textures;
extern int tex_cube_level;


};

#endif

#endif

