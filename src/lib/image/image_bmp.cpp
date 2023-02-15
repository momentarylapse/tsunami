#include "image.h"
#include <stdio.h>
#include "../os/file.h"
#include "../os/msg.h"

/*static const char *sys_str_f(const string &s)
{
	return s.c_str();
}*/

//--------------------------------------------------------------------------------------------------
// bmp files
//--------------------------------------------------------------------------------------------------



struct sImageColor
{
	unsigned char r,g,b,a;
};

inline void fill_image_color( Image &image, sImageColor *data, unsigned char *col, unsigned char *pal, int depth, int alpha_bits, bool pal_tga )
{
	// 8 bit (paletted tga/bmp)
	if (depth==8){
		if (pal_tga){
			data->r=pal[col[0]*3+2];
			data->g=pal[col[0]*3+1];
			data->b=pal[col[0]*3  ];
			data->a=255;
		}else{
			data->r=pal[col[0]*4+2];
			data->g=pal[col[0]*4+1];
			data->b=pal[col[0]*4  ];
			data->a=255;
		}
	}

	// 16 bit (just tga)
	if (depth==16){
		if (alpha_bits>0){
			data->r=(col[1]&252)*2;
			data->g=(col[1]&3)*64+(col[0]&224)/4;
			data->b=(col[0]&31)*8;
			data->a=(col[1]&128)/128*255;
		}else{
			data->r=(col[1]&252)*2;
			data->g=(col[1]&3)*64+(col[0]&224)/4;
			data->b=(col[0]&31)*8;
			data->a=255;
		}
	}

	// 24 bit (tga/bmp)
	if (depth==24){
		data->r=col[2];
		data->g=col[1];
		data->b=col[0];
		data->a=255;
	}

	// 32 bit (just tga)
	if (depth==32){
		data->r=col[2];
		data->g=col[1];
		data->b=col[0];
		data->a=col[3];
	}

	// color key test
	if (data->r == 0)
		if (data->g == 255)
			if (data->b == 0){
				// color == green?
				//   -> color := black    alpha := 0
				data->g = 0;
				data->a = 0;
				image.alpha_used = true;
			}
}

inline unsigned int get_int_from_buffer(unsigned char *buffer, int pos, int bytes)
{
	unsigned int r=0;
	/*for (int i=pos;i<pos+bytes;i++)
		r=r*256+buffer[i];*/
	for (int i=pos+bytes-1;i>=pos;i--)
		r=r*256+buffer[i];
	return r;
}

void image_load_bmp(const Path &filename, Image &image)
{
	//msg_write("bmp");
	unsigned char Header[56];
	unsigned char *pal = nullptr, temp_buffer[8];
	FILE* f = fopen(filename.str().c_str(), "rb");
	static_cast<void>(fread(&Header, 56, 1, f));

	image.width = get_int_from_buffer(Header, 18, 4);
	image.height = get_int_from_buffer(Header, 22, 4);
	bool reversed=true;
	if (image.height<0){
		image.height = 256 + image.height;
		reversed = false;
	}
	if (image.width<0){
		image.width = 256 + image.width;
		reversed = false;
	}
	int depth = get_int_from_buffer(Header,28,2);
	/*msg_write(image.Width);
	msg_write(image.Height);
	msg_write(depth);*/
	image.data.resize(image.width * image.height);
	int offset=get_int_from_buffer(Header,10,4);
	int bytes_per_row_o= image.width * depth / 8;
	int bytes_per_row = bytes_per_row_o+(4 - bytes_per_row_o % 4) % 4;
	int clr_used=get_int_from_buffer(Header,46,4);

	if (depth<16){
		if (clr_used==0)clr_used=1<<depth;
		//msg_write(clr_used);
		pal= new unsigned char[4*clr_used];
		fseek(f,-2,SEEK_CUR);
		static_cast<void>(fread(pal,4,clr_used,f));
	}

	fseek(f, offset, SEEK_SET);
	unsigned char *data = new unsigned char[bytes_per_row * image.height];
	//msg_write(bytes_per_row);
	if (reversed){
		//msg_write("Reversed!");
		for (int n=0;n<image.height;n++){
			static_cast<void>(fread(data + (bytes_per_row_o * n), sizeof(unsigned char), bytes_per_row_o, f));
			static_cast<void>(fread(temp_buffer, 1, bytes_per_row-bytes_per_row_o, f));
		}
	}else{
		//msg_write("nicht Reversed!");
		for (int n=image.height-1;n>=0;n--){
			static_cast<void>(fread(data + (bytes_per_row_o * n), sizeof(unsigned char), bytes_per_row_o, f));
			static_cast<void>(fread(temp_buffer, 1, bytes_per_row-bytes_per_row_o, f));
		}
	}

	switch(depth){
		case 24:
		case 8:
			for (int n=0;n<image.width * image.height;n++)
				fill_image_color(image, (sImageColor*)&image.data[n], data+n*(depth/8),pal,depth,0,false);
			break;
		default:
			msg_error(format("unbehandelte Farbtiefe: %d", depth));
	}
	if (pal)
		delete[]pal;
	delete[]data;
	fclose(f);
}

void image_save_bmp(const Path &filename, const Image &image) {
	try {
		auto f = os::fs::open(filename, "wb");
		image.set_mode(Image::Mode::RGBA);

		int row_size = 4 * (int)((image.width * 3 + 3) / 4);
		int data_size = row_size * image.height;

		unsigned char Header[56];
		memset(Header, 0, 56);
		Header[0] = 'B';
		Header[1] = 'M';
		*(int*)(&Header[2]) = 56 + data_size; // size
		*(int*)(&Header[10]) = 56; // data offset
		*(int*)(&Header[14]) = 40;
		*(int*)(&Header[18]) = image.width;
		*(int*)(&Header[22]) = image.height;
		*(short*)(&Header[26]) = 1; // # planes
		*(short*)(&Header[28]) = 24; // bits
		*(int*)(&Header[30]) = 0; // compression
		*(int*)(&Header[34]) = data_size;
		f->write(Header, 56);
		unsigned char *row = new unsigned char[row_size + 16];
		for (int y=image.height-1;y>=0;y--) {
			unsigned int *d = ((unsigned int*)image.data.data) + (y * image.width);
			unsigned char *p = row;
			for (int x=0;x<image.width;x++) {
				*(p ++) = (*d) >> 16;
				*(p ++) = (*d) >> 8;
				*(p ++) = (*d);
				d ++;
			}
			f->write(row, row_size);
		}
		delete[](row);

		delete f;
	} catch(...) {
	}
}
