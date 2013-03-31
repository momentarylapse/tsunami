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
#include "../types/types.h"

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

	void __init__();
	void __delete__();

	bool Empty(){	return (data.num == 0);	}

	void Load(const string &filename);
	void LoadFlipped(const string &filename);
	void Create(int width, int height, const color &c);
	void Save(const string &filename) const;
	void Delete();

	void Scale(int width, int height);
	void FlipV();
	void SetMode(int mode) const;
	void SetPixel(int x, int y, const color &c);
	color GetPixel(int x, int y) const;
	color GetPixelInterpolated(float x, float y) const;
};

#include "image_bmp.h"
#include "image_tga.h"
#include "image_jpg.h"

#endif
