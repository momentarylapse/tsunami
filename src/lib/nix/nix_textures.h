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
#include "../os/path.h"

namespace nix{

class Context;

void init_textures(Context* ctx);


class Texture : public Sharable<base::Empty> {
public:
	enum class Type {
		NONE,
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
	int width, height, depth, samples;
	bool valid;
	
	unsigned int texture;
	unsigned int internal_format;

	Texture();
	Texture(int width, int height, const string& format);
	~Texture();

	void _cdecl write(const Image& image);
	void write_with_color_space(const Image& image, ColorSpace color_space);
	void _cdecl read(Image& image) const;
	void _cdecl read_float(DynamicArray& data) const;
	void _cdecl write_float(const DynamicArray &data);
	void _cdecl reload();
	void _cdecl unload();

	int channels() const;


protected:
	void _release();
	void _create_2d(int width, int height, unsigned int format);
public:

	void _cdecl set_options(const string& op) const;

	static xfer<Texture> _cdecl load(const Path& filename);
};


class TextureMultiSample : public Texture {
public:
	TextureMultiSample(int width, int height, int samples, const string& format);
};

class VolumeTexture : public Texture {
public:
	VolumeTexture(int nx, int ny, int nz, const string& format);
};

class ImageTexture : public Texture {
public:
	ImageTexture(int width, int height, const string& format);
};

class DepthBuffer : public Texture {
public:
	DepthBuffer(int width, int height, const string& format);
};

class RenderBuffer : public Texture {
public:
	RenderBuffer(int width, int height, const string& format);
	RenderBuffer(int width, int height, int samples, const string& format);
};

class CubeMap : public Texture {
public:
	CubeMap(int size, const string& format);

	void _cdecl write_side(int side, const Image& image);
	void _cdecl fill_side(int side, Texture* source);
};


void bind_textures(const Array<Texture*>& textures);
void bind_texture(int binding, Texture* t);
void bind_image(int binding, Texture* t, int level, int layer, bool writable);

extern int tex_cube_level;


};

#endif

#endif

