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

	void _cdecl __init__();
	void _cdecl __delete__();

	bool _cdecl Empty(){	return (data.num == 0);	}

	void _cdecl Load(const string &filename);
	void _cdecl LoadFlipped(const string &filename);
	void _cdecl Create(int width, int height, const color &c);
	void _cdecl Save(const string &filename) const;
	void _cdecl Delete();

	void _cdecl Scale(int width, int height);
	void _cdecl FlipV();
	void _cdecl SetMode(int mode) const;
	void _cdecl SetPixel(int x, int y, const color &c);
	color _cdecl GetPixel(int x, int y) const;
	color _cdecl GetPixelInterpolated(float x, float y) const;
};

#endif
