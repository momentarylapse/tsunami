/*----------------------------------------------------------------------------*\
| Image                                                                        |
| -> loads and saves(?) graphic files                                          |
|                                                                              |
| last update: 2011.07.28 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _IMAGE_EXISTS_
#define _IMAGE_EXISTS_


#include "../00_config.h"
#include "../file/file.h"
#include "../types/types.h"

extern string ImageVersion;

class Image
{
	public:
	int width, height;
	Array<unsigned int> data;
	bool alpha_used;
	bool error;
	
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
};

/*void ImageLoadFlipped(Image &image, const string &filename);
void ImageLoad(Image &image, const string &filename);
void ImageCreate(Image &image, int width, int height, const color &c);
void ImageSave(const Image &image, const string &filename);
void ImageDelete(Image &image);

void ImageCopy(Image &dest, const Image &source);
void ImageScale(Image &image, int width, int height);
void ImageFlipV(Image &image);
void ImageSetPixel(Image &image, int x, int y, const color &c);
color ImageGetPixel(Image &image, int x, int y);*/

#include "image_bmp.h"
#include "image_tga.h"
#include "image_jpg.h"

#endif
