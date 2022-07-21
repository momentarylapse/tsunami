/*
 * image_png.cpp
 *
 *  Created on: 29.09.2013
 *      Author: michi
 */

#define HAS_LIB_ZLIB 1

#if HAS_LIB_ZLIB
#include "image.h"
#include <stdio.h>
#include "../os/file.h"
#include "../os/msg.h"

#include <zlib.h>

int endian_big_to_little(int i) {
	return ((i & 0xff) << 24) | ((i & 0xff00) << 8) | ((i & 0xff0000) >> 8) | ((i & 0xff000000) >> 24);
}

int read_int_big_endian(Stream *f) {
	unsigned char c[4];
	f->read(c, 4);
	return c[3] + (c[2] << 8) + (c[1] << 16) + (c[0] << 24);
}


static unsigned char png_paeth(int a, int b, int c) {
  int pa = abs(b - c);
  int pb = abs(a - c);
  int pc = abs(a + b - c - c);

  if ((pc < pa) and (pc < pb))
	  return c;
  if (pb < pa)
	  return b;
  return a;
}

void png_unfilter(unsigned char *cur, unsigned char *prev, int num, int stride, int type) {
	if (type == 0) {
	} else if (type == 1) {
		for (int i=stride; i<num; i++)
			cur[i] = cur[i] + cur[i - stride];
	} else if (type == 2) {
		for (int i=0; i<num; i++)
			cur[i] = cur[i] + prev[i];
	} else if (type == 3) {
		for (int i=0; i<stride; i++)
			cur[i] = cur[i] + prev[i] / 2;
		for (int i=stride; i<num; i++)
			cur[i] = cur[i] + (cur[i - stride] + prev[i]) / 2;
	} else if (type == 4) {
		for (int i=0; i<stride; i++)
			cur[i] = cur[i] + prev[i];
		for (int i=stride; i<num; i++)
			cur[i] = cur[i] + png_paeth(cur[i - stride], prev[i], prev[i - stride]);
	} else {
		msg_error("png: unhandled filter type: " + i2s(type));
	}
}

void image_load_png(const Path &filename, Image &image) {
	Stream *f = nullptr;
	try {
	f = os::fs::open(filename, "rb");

	// intro
	bytes buf = f->read(8);

	bytes data;

	int bytes_per_pixel = 1;

	while (!f->is_end()) {
		// read chunk
		int size = read_int_big_endian(f);//endian_big_to_little(f->ReadInt());
		string name = f->read(4);
		//msg_write(size);
		//msg_write("chunk: " + name);
		if (name == "IHDR") {
			int w = read_int_big_endian(f);
			int h = read_int_big_endian(f);
			image.create(w, h, Black);
			buf = f->read(2);
			int bits_per_channel = (unsigned char)buf[0];
			int type = (unsigned char)buf[1];
			// 0 = gray, 2 = rgb, 6 = rgba
			if (bits_per_channel != 8)
				throw string("unhandled bits per channel: " + i2s(bits_per_channel));
			if (type == 2) {
				bytes_per_pixel = 3;
			} else if (type == 6) {
				bytes_per_pixel = 4;
				image.alpha_used = true;
			} else {
				throw string("unhandled color type: " + i2s(type));
			}

			f->seek(size - 10);
		} else if (name == "IDAT") {
			int size0 = data.num;
			data.resize(size0 + size);
			f->read(&data[size0], size);
		} else if (name == "IEND") {
			break;
		} else {
			f->seek(size);
		}
		f->read(4); // crc
	}

	bytes dest;
	dest.resize(image.height * (image.width * bytes_per_pixel + 2) + 1024);
	unsigned long len = dest.num;
	int r = uncompress((unsigned char*)&dest[0], &len, (unsigned char*)&data[0], data.num);
	if (r != 0)
		throw string("uncompress");
	/*msg_write(data.num);
	msg_write(len);
	msg_write(r);*/

	int bytes_per_line = image.width * bytes_per_pixel;

	for (int y=0; y<image.height; y++) {
		int i0 = y * (bytes_per_line + 1);
		int filter_type = dest[i0];

		unsigned char *l_cur = (unsigned char*)&dest[i0 + 1];
		unsigned char *l_prev = (unsigned char*)&dest[i0 - bytes_per_line];
		if (i0 == 0)
			l_prev = l_cur;
		png_unfilter(l_cur, l_prev, bytes_per_line, bytes_per_pixel, filter_type);

		int i = 0;
		for (int x=0; x<image.width; x++) {
			float r = l_cur[i ++] / 255.0f;
			float g = l_cur[i ++] / 255.0f;
			float b = l_cur[i ++] / 255.0f;
			float a = 1;
			if (bytes_per_pixel > 3)
				a = l_cur[i ++] / 255.0f;
			image.set_pixel(x, image.height - y - 1, color(a, r, g, b));
		}
	}
	delete f;

	} catch(os::fs::FileError &e) {
		msg_error("png: " + e.message());
		if (f)
			delete f;
	} catch(string &s) {
		msg_error("png: " + s);
		if (f)
			delete f;
	}
}

#else
void image_load_png(const string &filename, Image &image) {}
#endif
