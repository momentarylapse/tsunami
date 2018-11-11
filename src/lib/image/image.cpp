#include "image.h"
#include "../file/file.h"
#include "../math/math.h"

#include "image_bmp.h"
#include "image_tga.h"
#include "image_jpg.h"
#include "image_png.h"

#include "ImagePainter.h"


string ImageVersion = "0.2.5.0";

Image::Image()
{
	width = height = 0;
	error = false;
	mode = ModeRGBA;
	alpha_used = false;
}

Image::Image(int _width, int _height, const color &c)
{
	create(_width, _height, c);
}

void Image::__init__()
{
	new(this) Image;
}

void Image::__init_ext__(int _width, int _height, const color &c)
{
	new(this) Image(_width, _height, c);
}

void Image::__delete__()
{
	this->Image::~Image();
}

// mode: rgba
//    = r + g<<8 + b<<16 + a<<24
void Image::load_flipped(const string &filename)
{
	// reset image
	width = 0;
	height = 0;
	mode = ModeRGBA;
	error = false;
	alpha_used = false;
	data.clear();

	// file ok?
	if (!file_test_existence(filename)){
		msg_error("Image.Load: file does not exist: " + filename);
		return;
	}
	
	string ext = filename.extension();
	
	if (ext == "bmp")
		image_load_bmp(filename, *this);
	else if (ext == "tga")
		image_load_tga(filename, *this);
	else if (ext == "jpg")
		image_load_jpg(filename, *this);
	else if (ext == "png")
		image_load_png(filename, *this);
	else
		msg_error("ImageLoad: unhandled file extension: " + ext);
}

void Image::load(const string &filename)
{
	load_flipped(filename);
	flip_v();
}

inline unsigned int image_color_rgba(const color &c)
{
	return int(c.r * 255.0f) + (int(c.g * 255.0f) << 8) + (int(c.b * 255.0f) << 16) + (int(c.a * 255.0f) << 24);
}

inline unsigned int image_color_bgra(const color &c)
{
	return int(c.b * 255.0f) + (int(c.g * 255.0f) << 8) + (int(c.r * 255.0f) << 16) + (int(c.a * 255.0f) << 24);
}

inline color image_uncolor_rgba(unsigned int i)
{
	return color((float)(i >> 24) / 255.0f,
	              (float)(i & 255) / 255.0f,
	              (float)((i >> 8) & 255) / 255.0f,
	              (float)((i >> 16) & 255) / 255.0f);
}

inline color image_uncolor_bgra(unsigned int i)
{
	return color((float)(i >> 24) / 255.0f,
	              (float)((i >> 16) & 255) / 255.0f,
	              (float)((i >> 8) & 255) / 255.0f,
	              (float)(i & 255) / 255.0f);
}

void Image::create(int _width, int _height, const color &c)
{
	// create
	width = _width;
	height = _height;
	mode = ModeRGBA;
	error = false;
	alpha_used = (c.a != 1.0f);
	
	// fill image
	data.resize(width * height);
	unsigned int ic = image_color_rgba(c);
	for (int i=0;i<data.num;i++)
		data[i] = ic;
}

void Image::save(const string &filename) const
{
	string ext = filename.extension();
	if (ext == "tga")
		image_save_tga(filename, *this);
	else if (ext == "bmp")
		image_save_bmp(filename, *this);
	else
		msg_error("ImageSave: unhandled file extension: " + ext);
}

void Image::clear()
{
	width = 0;
	height = 0;
	data.clear();
}

Image* Image::scale(int _width, int _height) const
{
	Image *r = new Image(_width, _height, Black);

	for (int x=0;x<_width;x++)
		for (int y=0;y<_height;y++){
			int x0 = (int)( (float)x * (float)width / (float)_width );
			int y0 = (int)( (float)y * (float)height / (float)_height );
			r->data[y * _width + x] = data[y0 * width + x0];
		}

	return r;
}

void Image::flip_v()
{
	unsigned int t;
	unsigned int *d = &data[0];
	for (int y=0;y<height/2;y++)
		for (int x=0;x<width;x++){
			t = d[x + y * width];
			d[x + y * width] = d[x + (height - y - 1) * width];
			d[x + (height - y - 1) * width] = t;
		}
}

inline void col_conv_rgba_to_bgra(unsigned int &c)
{
	unsigned int a = (c & 0xff000000) >> 24;
	if (a < 255){
		// cairo wants pre multiplied alpha
		float aa = (float)((c & 0xff000000) >> 24) / 255.0f;
		unsigned int r = (unsigned int)((float)(c & 0xff) * aa);
		unsigned int g = (unsigned int)((float)((c & 0xff00) >> 8) * aa);
		unsigned int b = (unsigned int)((float)((c & 0xff0000) >> 16) * aa);
		c = (c & 0xff000000) + b + (r << 16) + (g << 8);
	}else{
		unsigned int r = (c & 0xff);
		unsigned int b = (c & 0xff0000) >> 16;
		c = (c & 0xff00ff00) + b + (r << 16);
	}
}

inline void col_conv_bgra_to_rgba(unsigned int &c)
{
	unsigned int r = (c & 0xff0000) >> 16;
	unsigned int b = (c & 0xff);
	c = (c & 0xff00ff00) + r + (b << 16);
}

void Image::set_mode(int _mode) const
{
	if (_mode == mode)
		return;

	unsigned int *c = (unsigned int*)data.data;
	if (mode == ModeRGBA){
		if (_mode == ModeBGRA){
			for (int i=0;i<data.num;i++)
				col_conv_rgba_to_bgra(*(c ++));
		}
	}else if (mode == ModeBGRA){
		if (_mode == ModeRGBA){
			for (int i=0;i<data.num;i++)
				col_conv_bgra_to_rgba(*(c ++));
		}
	}
	mode = _mode;
}


void Image::set_pixel(int x, int y, const color &c)
{
	if ((x >= 0) and (x < width) and (y >= 0) and (y < height)){
		if (mode == ModeBGRA)
			data[x + y * width] = image_color_bgra(c);
		else if (mode == ModeRGBA)
			data[x + y * width] = image_color_rgba(c);
	}
}

void Image::draw_pixel(int x, int y, const color &c)
{
	if (c.a >= 1){
		set_pixel(x, y, c);
	}else if (c.a <= 0){
	}else{
		color c0 = get_pixel(x, y);
		set_pixel(x, y, ColorInterpolate(c0, c, c.a));
	}
}

color Image::get_pixel(int x, int y) const
{
	if ((x >= 0) and (x < width) and (y >= 0) and (y < height)){
		if (mode == ModeBGRA)
			return image_uncolor_bgra(data[x + y * width]);
		else if (mode == ModeRGBA)
			return image_uncolor_rgba(data[x + y * width]);
	}
	return Black;
}

// use bilinear interpolation
//  x repeats in [0..width)
//  y repeats in [0..height)
//  each pixel has its maximum at offset (0.5, 0.5)
color Image::get_pixel_interpolated(float x, float y) const
{
	x = loopf(x, 0, (float)width);
	y = loopf(y, 0, (float)height);
	int x0 = (x >= 0.5f) ? (int)(x - 0.5f) : (width - 1);
	int x1 = (x0 < width - 1) ? (x0 + 1) : 0;
	int y0 = (y >= 0.5f) ? (int)(y - 0.5f) : (height - 1);
	int y1 = (y0 < height- 1) ? (y0 + 1) : 0;
	color c00 = get_pixel(x0, y0);
	color c01 = get_pixel(x0, y1);
	color c10 = get_pixel(x1, y0);
	color c11 = get_pixel(x1, y1);
	float sx = loopf(x + 0.5f, 0, 1);
	float sy = loopf(y + 0.5f, 0, 1);
	// bilinear interpolation
	return (c00 * (1 - sy) + c01 * sy) * (1 - sx) + (c10 * (1 - sy) + c11 * sy) * sx;
}

Image *LoadImage(const string &filename)
{
	Image *im = new Image;
	im->load(filename);
	if (!im->error)
		return im;
	delete(im);
	return nullptr;
}

