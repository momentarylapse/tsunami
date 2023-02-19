/*----------------------------------------------------------------------------*\
| Image                                                                        |
| -> loads and saves(?) graphic files                                          |
|                                                                              |
| last update: 2011.07.28 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _IMAGE_EXISTS_
#define _IMAGE_EXISTS_


#include "../base/base.h"
#include "../base/pointer.h"
#include "color.h"

class Painter;
class Path;

class Image {
public:
	
	enum class Mode {
		// little endian OpenGL convention
		RGBA, // 0xrr 0xgg 0xbb 0xaa = 0xaabbggrr
		BGRA, // 0xbb 0xgg 0xrr 0xaa = 0xaarrggbb
	};

	int width, height;
	mutable Mode mode;
	Array<unsigned int> data;
	bool alpha_used;
	bool error;
	Image();
	Image(int width, int height, const color &c);

	void _cdecl __init__();
	void _cdecl __init_ext__(int width, int height, const color &c);
	void _cdecl __delete__();

	void _cdecl __assign__(const Image &other){ *this = other; }

	bool _cdecl is_empty(){	return (data.num == 0);	}

	static xfer<Image> load(const Path &filename);
	void _cdecl _load(const Path &filename);
	void _cdecl _load_flipped(const Path &filename);
	void _cdecl create(int width, int height, const color &c);
	void _cdecl save(const Path &filename) const;
	void _cdecl clear();

	xfer<Image> _cdecl scale(int width, int height) const;
	void _cdecl flip_v();
	void _cdecl set_mode(Mode mode) const;
	void _cdecl set_pixel(int x, int y, const color &c);
	void _cdecl draw_pixel(int x, int y, const color &c);
	color _cdecl get_pixel(int x, int y) const;
	color _cdecl get_pixel_interpolated(float x, float y) const;

	xfer<Painter> start_draw();
};


#endif
