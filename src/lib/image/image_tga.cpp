#include "image.h"
#include <stdio.h>
#include "../file/file.h"

//--------------------------------------------------------------------------------------------------
// tga files
//--------------------------------------------------------------------------------------------------



struct sImageColor
{
	unsigned char r,g,b,a;
};

inline void __fill_image_color( Image &image, sImageColor *data, unsigned char *col, unsigned char *pal, int depth, int alpha_bits, bool pal_tga )
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

void image_load_tga(const string &filename, Image &image)
{
	//msg_write("tga");
	unsigned char Header[18];
	int x;
	unsigned char *data=nullptr,*pal=nullptr;
	FILE* f=fopen(filename.sys_filename().c_str(),"rb");
	int r=fread(&Header, 18, 1, f);
	int offset=get_int_from_buffer(Header,0,1)+18;
	int tga_type=get_int_from_buffer(Header,2,1);
	//msg_write(tga_type);

	image.width = get_int_from_buffer(Header,12,2);
	if (image.width < 0)
		image.width= - image.width;
	image.height = get_int_from_buffer(Header,14,2);
	if (image.height < 0)
		image.height = -image.height;
	int depth = get_int_from_buffer(Header,16,1);
	int alpha_bits = get_int_from_buffer(Header,17,1);
	image.alpha_used = alpha_bits > 0;
	bool compressed=((tga_type&8)>0);
	/*if (compressed)	msg_write("komprimiert");
	else			msg_write("unkomprimiert");
	msg_write("Breite: " + i2s(image.width));
	msg_write("Hoehe: " + i2s(image.height));
	msg_write("Farbtiefe: " + i2s(depth));
	msg_write("Alpha-Bits: " + i2s(alpha_bits));*/
	image.data.resize(image.width * image.height);
	int size = image.width * image.height;
	int bpp = depth / 8;

	fseek(f,offset,SEEK_SET);

	if (depth<16){
		pal=new unsigned char[3*256];
		r=fread(pal,3,256,f);
	}

	if (compressed){
		int currentpixel=0;
		unsigned char colorbuffer[4];
		switch(depth){
			case 32:
			case 24:
			case 16:
			case 8:
				do{
					unsigned char chunkheader = 0;
					r=fread(&chunkheader, 1, 1, f);
					if ((chunkheader & 0x80) != 0x80){
						chunkheader++;
						//msg_write(string("raw ",i2s(chunkheader)));
						while (chunkheader-- > 0){
							r=fread(colorbuffer, 1, bpp, f);
							__fill_image_color(image, (sImageColor*)&image.data[currentpixel], colorbuffer, pal, depth, alpha_bits, true);
							currentpixel++;
						}
					}else{
						chunkheader = (unsigned char)((chunkheader & 0x7F) + 1);
						//msg_write(string("rle ",i2s(chunkheader)));
						r=fread(colorbuffer, 1, bpp, f);
						while(chunkheader-- > 0){
							__fill_image_color(image, (sImageColor*)&image.data[currentpixel], colorbuffer, pal, depth, alpha_bits, true);
							currentpixel++;
						}
					}
				}while(currentpixel < size);
				break;
			default:
				msg_error(format("NixLoadTGA: unsupported color depth (compressed): %d", depth));
				break;
		}
	}else{
		// uncompressed
		switch(depth){
			case 32:
			case 24:
			case 16:
			case 8:
				data=new unsigned char[bpp*size];
				r=fread(data,bpp,size,f);
				for (x=0;x<image.width*image.height;x++)
					__fill_image_color(image, (sImageColor*)&image.data[x], data + x * bpp, pal, depth, alpha_bits, true);
				break;
			default:
				msg_error(format("image_load_tga: unsupported color depth (uncompressed): %d", depth));
				break;
		}
	}
	if (pal)
		delete[]pal;
	if (data)
	    delete[]data;
    fclose(f);
}

// encode tga files via run-length-encoding
//#define SAVE_TGA_RLE

// data must be   r-g-b-a
// 1 unsigned char per color channel  ( r8 g8 b8 a8 )
// first pixel is top left, the second one leads right
// tga-formats:
//    bits=32	alpha_bits=?	->	a8r8g8b8
//    bits=24	alpha_bits=?	->	a0r8g8b8
//    bits=16	alpha_bits=0	->	a0r5g5b5
//    bits=16	alpha_bits=1	->	a1r5g5b5
//    bits=16	alpha_bits=4	->	a4r4g4b4
static unsigned char Header[18];
//void NixSaveTGA(const string &filename,int width,int height,int bits,int alpha_bits,void *data)

void image_save_tga(const string &filename, const Image &image)
{
	FILE* f=fopen(filename.sys_filename().c_str(),"wb");
	if (!f){
		msg_error("couldn't save tga file: " + filename);
		return;
	}
	image.set_mode(Image::ModeRGBA);
	int bits = image.alpha_used ? 32 : 24;
	int alpha_bits = image.alpha_used ? 8 : 0;
	memset(Header,0,18);
#ifdef SAVE_TGA_RLE
	Header[2]=10;
#else
	Header[2]=2;
#endif
	Header[12] = image.width%256;
	Header[13] = image.width/256;
	Header[14] = image.height%256;
	Header[15] = image.height/256;
	Header[16] = bits;
	Header[17] = alpha_bits;
	int rw=fwrite(&Header, 18, 1, f);
	unsigned int color,last_color=0;
	unsigned char *_data=(unsigned char*)image.data.data;
	int a,r,g,b;
	bool rle_even=false;
	int rle_num=-1;
	for (int y=image.height-1;y>=0;y--)
		for (int x=0;x<image.width;x++){
			int offset=(x+y*image.width)*4;
#ifdef SAVE_TGA_RLE
			if (rle_num<0){
				int os1=offset;
				int os2=offset+4;//((x+4)%image.width+(y-(x+4)/image.width)*image.width)*4;
				rle_even=(*(int*)&_data[os1]==*(int*)&_data[os2]);
				for (rle_num=0;rle_num<128;rle_num++){
					os1=offset+rle_num*4  ;//((x+rle_num*4+4)%image.width+(y-(x+rle_num*4+4)/image.width)*image.width)*4;
					os2=offset+rle_num*4+4;//((x+rle_num*4+8)%image.width+(y-(x+rle_num*4+8)/image.width)*image.width)*4;
					if (rle_even){
						if (*(int*)&_data[os1]!=*(int*)&_data[os2])
							break;
					}else{
						if (*(int*)&_data[os1]==*(int*)&_data[os2]){
							rle_num--;
							break;
						}
					}
				}
				if (rle_even)
					msg_write("even");
				else
					msg_write("raw");
				msg_write(rle_num);
				unsigned char rle_header=128+rle_num;
				rw=fwrite(&rle_header, 1, 1, f);
			}
			msg_write(*(int*)&_data[offset]);
			rle_num--;
			if ((rle_even)&&(rle_num>=0))
				continue;
#endif
			r=_data[offset  ];
			g=_data[offset+1];
			b=_data[offset+2];
			a=_data[offset+3];
			switch (bits){
				case 32:
					color=b + g*256 + r*65536 + a*16777216;
					break;
				case 24:
					if (alpha_bits==0)
						color=b + g*256 + r*65536;
					break;
				case 16:
					if (alpha_bits==4)
						color =int(b/16) + int(g/16)*16 + int(r/16)*256 + int(a/16)*4096;
					else
						color =int(b/8)  + int(g/8)*32  + int(r/8)*1024 + int(a/128)*32768;
					break;
			}
			rw=fwrite(&color, bits/8, 1, f);
			//msg_write(color);
		}
    fclose(f);
}
