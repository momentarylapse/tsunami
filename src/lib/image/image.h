/*----------------------------------------------------------------------------*\
| Image                                                                        |
| -> loads and saves(?) graphic files                                          |
|                                                                              |
| last update: 2011.07.28 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _IMAGE_EXISTS_
#define _IMAGE_EXISTS_


#include "../00_config.h"
#include "../base/base.h"
#include "../types/types.h"

extern string ImageVersion;

class Image
{
	public:
	int width, height;
	Array<unsigned int> data;
	bool alpha_used;
	bool error;
	
	void __init__();
	void __delete__();

	bool Empty(){	return (data.num == 0);	}

	void Load(const string &filename);
	void LoadFlipped(const string &filename);
	void Create(int width, int height, const color &c);
	void Save(const string &filename) const;
	void Delete();

	void CopyTo(Image &dest) const;
	void Scale(int width, int height);
	void FlipV();
	void SetPixel(int x, int y, const color &c);
	color GetPixel(int x, int y) const;
	color GetPixelInterpolated(float x, float y) const;
};

#include "image_bmp.h"
#include "image_tga.h"
#include "image_jpg.h"

#endif
