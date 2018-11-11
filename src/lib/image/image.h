/*----------------------------------------------------------------------------*\
| Image                                                                        |
| -> loads and saves(?) graphic files                                          |
|                                                                              |
| last update: 2011.07.28 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _IMAGE_EXISTS_
#define _IMAGE_EXISTS_


#include "../config.h"
#include "../base/base.h"
#include "color.h"

extern string ImageVersion;

class ImagePainter;

class Image
{
	public:
	int width, height;
	mutable int mode;
	Array<unsigned int> data;
	bool alpha_used;
	bool error;
	
	enum{
		// little endian OpenGL convention
		ModeRGBA, // 0xrr 0xgg 0xbb 0xaa = 0xaabbggrr
		ModeBGRA, // 0xbb 0xgg 0xrr 0xaa = 0xaarrggbb
	};
	Image();
	Image(int width, int height, const color &c);

	void _cdecl __init__();
	void _cdecl __init_ext__(int width, int height, const color &c);
	void _cdecl __delete__();

	void _cdecl __assign__(const Image &other){ *this = other; }

	bool _cdecl is_empty(){	return (data.num == 0);	}

	void _cdecl load(const string &filename);
	void _cdecl load_flipped(const string &filename);
	void _cdecl create(int width, int height, const color &c);
	void _cdecl save(const string &filename) const;
	void _cdecl clear();

	Image* _cdecl scale(int width, int height) const;
	void _cdecl flip_v();
	void _cdecl set_mode(int mode) const;
	void _cdecl set_pixel(int x, int y, const color &c);
	void _cdecl draw_pixel(int x, int y, const color &c);
	color _cdecl get_pixel(int x, int y) const;
	color _cdecl get_pixel_interpolated(float x, float y) const;
};

// windows.h hack...
#ifdef LoadImage
#undef LoadImage
#endif

Image* _cdecl LoadImage(const string &filename);

#endif
