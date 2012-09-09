#include "image.h"
#include "../file/file.h"

static Image EmptyImage;

//--------------------------------------------------------------------------------------------------
// jpg files
//--------------------------------------------------------------------------------------------------

Image *cur_image;

struct s_jpg_huffman_table
{
	unsigned char bits[16];
	unsigned char value[4096];
	int min[16],max[16];
};

static s_jpg_huffman_table jpg_htac[4],jpg_htdc[4];

static float jpg_qt[4][64];

static unsigned char jpg_zz[64]={
	 0, 1, 5, 6,14,15,27,28,
	 2, 4, 7,13,16,26,29,42,
	 3, 8,12,17,25,30,41,43,
	 9,11,18,24,31,40,44,53,
	10,19,23,32,39,45,52,54,
	20,22,33,38,46,51,55,60,
	21,34,37,47,50,56,59,61,
	35,36,48,49,57,58,62,63
};

struct s_jpg_color_info
{
	int ac[3],dc[3]; // huffman table ac/dc
	int v[3],h[3]; // vertical/horizontal sampling
	int q[3]; // quantization table
};

static int jpg_temp_bits=0;

static float jpg_cos_table[8][8];

int jpg_load_huffman_table(unsigned char *b,s_jpg_huffman_table *h)
{
	// length
	for (int i=0;i<16;i++)
		h->bits[i]=(*(b++));

	// values
	int nv=0;
	for (int i=0;i<16;i++){
		for (int j=0;j<h->bits[i];j++){
			//msg_write(i*256+j);
			//msg_write( h->value[i*256+j]=(*(b++)) );
			h->value[i*256+j]=(*(b++));
			nv++;
		}
	}

	// pre process for searching faster
	int t=0;
	for (int i=0;i<16;i++){
		h->min[i]=(h->bits[i]==0)?0xffff:t;
		t+=h->bits[i];
		h->max[i]=t-1;
		//msg_write(string2("min=%d  max=%d",h->min[i],h->max[i]));
		t*=2;
	}
	return nv+16;
}

int jpg_load_quantization_table(unsigned char *b,float *q)
{
	for (int i=0;i<64;i++)
		q[i]= (float)b[jpg_zz[i]];
	return 64;
}

inline int jpg_get_bits(int bits,bool sign)
{
	if (sign)
		if ( (jpg_temp_bits & 0x8000) == 0 )
			return (jpg_temp_bits>>(16-bits)) + 1 + (0xffffffff << bits);
	return jpg_temp_bits>>(16-bits);
}

inline void jpg_update_bits(unsigned char *&b,int &bit_off)
{
	// correct bit shifting over byte boundaries
	if (bit_off>=16){
		bit_off-=16;
		if ( ((b[-1]==0xff)&&(b[0]==0x00)) || ((b[0]==0xff)&&(b[1]==0x00)) ){
			b+=3;
			//msg_write("+=3     (0xff00) 2");
		}else{
			b+=2;
			//msg_write("+=2");
		}
	}else if (bit_off>=8){
		bit_off-=8;
		if ((b[-1]==0xff)&&(b[0]==0x00)){
			b+=2;
			//msg_write("+=2     (0xff00) 1");
		}else{
			b++;
			//msg_write("+=1");
		}
	}
	// convert 0xff00 to 0xff  m(T_T)m
	unsigned char *a=b,b0,b1,b2;
	if ((a[-1]==0xff)&&(a[0]==0x00))
		a++;
	b0=a[0];
	if ((a[0]==0xff)&&(a[1]==0x00))
		a++;
	b1=a[1];
	if ((a[1]==0xff)&&(a[2]==0x00))
		a++;
	b2=a[2];
	// join...but in "wrong" order!  m(T_T)m
	jpg_temp_bits=( ( ( (b0<<16) + (b1<<8) + b2) << bit_off ) >> 8 ) & 0x0000ffff;
	//msg_write(d2h(&jpg_temp_bits,3,true));
}

inline unsigned char jpg_get_huffman(s_jpg_huffman_table *h,unsigned char *&b,int &bit_off)
{
	jpg_update_bits(b,bit_off);
	//msg_write(d2h(&jpg_temp_bits,3,true));
	for (int i=0;i<16;i++){
		int c=jpg_get_bits(i+1,false);
		if ((c>=h->min[i])&&(c<=h->max[i])){
			//msg_write(i*256+(c-h->min[i]));
			//msg_write(string2("gefunden!   ii=%d   i=%d   c=%d   min=%d   v=%d",htii,i,c,h->min[i],h->value[i*256+(c-h->min[i])]));
			bit_off+=i+1;
			return h->value[i*256+(c-h->min[i])];
		}
	}
	msg_write("huffman :~~(");
	return 0;
}

void jpg_decode_huffman(int ac,int dc,unsigned char *&b,int &bit_off,int &prev,int *coeff)
{
	//msg_write("huffman");
	int coeff2[80];
	memset(coeff2,0,4*64);

	unsigned char *b0=b;

	// "dc"
	int size=jpg_get_huffman(&jpg_htdc[dc],b,bit_off);
	//msg_write(size);
	jpg_update_bits(b,bit_off);
	coeff2[0]=jpg_get_bits(size,true)+prev;
	prev=coeff2[0];
	//msg_write(coeff2[0]);
	bit_off+=size;

	// "ac"
	//msg_write("ac");
	for (int i=1;i<64;i++){
		unsigned char v=jpg_get_huffman(&jpg_htac[ac],b,bit_off);
	//msg_write(v);
		int size=(v & 0x0f);
		int n=(v >> 4);
		if (size==0){
			if (n==0)
				break;
			if (n==0x0f)
				i+=15;
		}else{
			i+=n; // zeroes!
			jpg_update_bits(b,bit_off);
			//msg_write(i);
			coeff2[i]=jpg_get_bits(size,true);
			//msg_write(coeff2[i]);
			bit_off+=size;
		}
	}
	//msg_write("zzz1");
	for (int i=0;i<64;i++)
		coeff[i]=coeff2[jpg_zz[i]];
	//msg_write("zzz2");
}

inline void jpg_cosine_retransform(int *coeff_in,int q,float *coeff_out)
{
	// de-quantizing....
	float coeff_in_f[64];
	float *qt=jpg_qt[q];
	for (int i=0;i<64;i++)
		coeff_in_f[i]=(float)coeff_in[i]*qt[i];

//	float inv_sqrt2=1.0f/sqrt(2.0f);

// M[n][x] = sum(m) coeff_in_f[n][m] * jpg_cos_table[x][m]
	float M[8][8];
	for (int n=0;n<8;n++)
		for (int x=0;x<8;x++){
			M[n][x] = 0;
			for (int m=0;m<8;m++)
				M[n][x] += coeff_in_f[n * 8 + m] * jpg_cos_table[x][m];
		}

// coeff_out[y][x] = sum(n) M[n][x] * jpg_cos_table[y][n]
	for (int x=0;x<8;x++)
		for (int y=0;y<8;y++){
			float f = 0;
			for (int n=0;n<8;n++)
				f += M[n][x] * jpg_cos_table[y][n];
			coeff_out[y * 8 + x] = 0.25f * f;
		}

	// cos transformation
/*	for (int x=0;x<8;x++)
		for (int y=0;y<8;y++){
			float f=0;
			for (int n=0;n<8;n++)
				for (int m=0;m<8;m++){
					float g = coeff_in_f[n*8+m]*jpg_cos_table[x][m]*jpg_cos_table[y][n];
//					if (m==0)	g*=inv_sqrt2;
//					if (n==0)	g*=inv_sqrt2;
					f+=g;
				}
		}*/
}

// combine some 8x8 blocks into a meta block (using sub sampling)
inline void jpg_combine_blocks(float *block,int &sx,int &sy,int &sh,int &sv,int &i,int &j,float *col)
{
	int fh=sx/sh/8;
	int fv=sy/sv/8;
	for (int x=0;x<8;x++){
		int x0=(x+i*8)*fh;
		for (int y=0;y<8;y++){
			float c=block[y*8+x];
			int y0=(y+j*8)*fv;
			// sub sampling
			for (int yy=0;yy<fv;yy++){
				float *tcol=&col[(y0+yy)*sx + x0];
				for (int xx=0;xx<fh;xx++){
					*tcol=c;
					tcol++;
				}
			}
		}
	}
}

#define _color_clamp_(c)\
	( (c<0.0f) ? 0.0f : ( (c>255.0f) ? 255.0f : c ) )

inline void jpg_insert_into_image(float *col0,float *col1,float *col2,int &sx,int &sy,int &x,int &y)
{
	for (int i=0;i<sx;i++){
		// "real position" in the image
		int _y = cur_image->height-(i+sy*y+1);
		int _x0=sx*x;
		unsigned char *d = (unsigned char*)&cur_image->data[ _y*cur_image->width + _x0 ];
		for (int j=0;j<sy;j++){
			// within "real image"?
			if (_x0+j<cur_image->width)
				if (_y>=0){
					// retransform color space YCbCr -> RGB
					col0[0]+=128.0f;
					float r=_color_clamp_(*col0                     + 1.402f    * *col2);
					float g=_color_clamp_(*col0 - 0.344136f * *col1 - 0.714136f * *col2);
					float b=_color_clamp_(*col0 + 1.772f    * *col1                    );
					// insert
					*(d++)=(int)r;
					*(d++)=(int)g;
					*(d++)=(int)b;
					*(d++)=255;
				}
			col0++;
			col1++;
			col2++;
		}
	}
}

#define _max(a,b)	( (a>b) ? a  : b )

void jpg_decode(unsigned char *b,s_jpg_color_info ci)
{
	unsigned char *a=b;
	//msg_write("start decode!");
	// size of meta blocks
	int sx=_max(ci.h[0],_max(ci.h[1],ci.h[2]))*8;
	int sy=_max(ci.v[0],_max(ci.v[1],ci.v[2]))*8;
	//msg_write(string2("%d x %d",sx,sy));
	// number of meta blocks in the image
	int nx = (cur_image->width +(sx-1))/sx;
	int ny = (cur_image->height+(sy-1))/sy;
	//msg_write(string2("%d x %d",nx,ny));
	// image dimensions in file (including "nonsense")
	int iw=nx*sx;
	int ih=ny*sy;

	// temporary data
	int coeff[64]; // block
	float col_block[64];
	float *col[3]; // meta block
	col[0]=new float[sx*sy];
	col[1]=new float[sx*sy];
	col[2]=new float[sx*sy];

	for (int i=0;i<8;i++)
		for (int j=0;j<8;j++)
			jpg_cos_table[i][j] = (float)cos( (2*i+1)*j*pi/16.0f ) * ((j == 0) ? 1.0f / sqrt(2.0f) : 1.0f);


/*CFile *f=new CFile();
f->Create("test");
f->SetBinaryMode(true);*/

	// read data
	int bit_off=0;
	int prev[3]={0,0,0};
	// meta blocks
	for (int y=0;y<ny;y++)
		for (int x=0;x<nx;x++){
			// 3 colors
			for (int c=0;c<3;c++){
				// sub blocks
				for (int i=0;i<ci.h[c];i++)
					for (int j=0;j<ci.v[c];j++){
						jpg_decode_huffman(ci.ac[c],ci.dc[c],b,bit_off,prev[c],coeff);
						jpg_cosine_retransform(coeff,ci.q[c],col_block);
						jpg_combine_blocks(col_block,sx,sy,ci.h[c],ci.v[c],j,i,col[c]);
					}
			}
			jpg_insert_into_image(col[0],col[1],col[2],sx,sy,x,y);
		}

		//f->WriteBuffer(image.data,image.width*image.height*3);

/*f->Close();
delete(f);*/

	delete[](col[0]);
	delete[](col[1]);
	delete[](col[2]);
}

void image_load_jpg(const string &filename, Image &image)
{
	CFile *f = OpenFileSilent(filename);
	if (!f)
		return;
	f->SetBinaryMode(true);
	string tt = f->ReadComplete();
	unsigned char *buf = (unsigned char*)tt.data;
	FileClose(f);

	unsigned char *b=buf;

	image.width = -1;
	image.height = -1;
	cur_image = &image;

	s_jpg_color_info ci;

	while(true){

		// read segments
		int seg_len=0;
		//msg_write("-------------------------");
		//msg_write(string2("%s (off:%d)",d2h(b,2,false),(int)b-(int)buf));
		if (b[0]==0xff){
			if (b[1]==0xd8){		// StartOfImage
				b+=2;
				//msg_write("SOI");
			}else if (b[1]==0xd9){	// EndOfImage
				//msg_write("EOI");
				break;
			}else if (b[1]==0xe0){	// JFIF-Tag
				//msg_write("APP0");
				seg_len=b[2]*256+b[3]-2;
				b+=4;
				b+=seg_len;
			}else if (b[1]==0xdb){	// QuantifyingTable
				//msg_write("DQT");
				seg_len=b[2]*256+b[3]-2;
				b+=4;
				int t=0;
				while(t<seg_len){
					int nt=(b[t] & 0x0f); // table index
					t+=jpg_load_quantization_table(&b[t+1],jpg_qt[nt])+1;
				}
				b+=seg_len;
			}else if (b[1]==0xc0){	// dimensions/color sampling
				//msg_write("SOF0");
				seg_len=b[2]*256+b[3]-2;
				b+=4;
				int bpp=b[0];
				image.height = b[1]*256 + b[2];
				image.width = b[3]*256 + b[4];
				//msg_write(string2("jpg: %d x %d, depth=%d",NixImage.width,NixImage.height,bpp*3));
				if (bpp!=8){	msg_error("jpg: depth!=24 unsupported!");	break;	}
				image.data.resize(image.width * image.height);
				int nc=b[5];
				if (nc!=3){
					msg_error("jpg: number of colors != 3 (unsupported)");
					break;
				}
				for (int i=0;i<3;i++){
					int c=b[i*3+6];
					ci.h[c-1]=b[i*3+7]>>4;
					ci.v[c-1]=b[i*3+7]&0x0f;
					ci.q[c-1]=b[i*3+8];
				}
				b+=seg_len;
			}else if (b[1]==0xc4){	// HuffmanTable
				//msg_write("DHT");
				seg_len=b[2]*256+b[3]-2;
				b+=4;
				int t=0;
				while(t<seg_len){
					int nt=(b[t] & 0x0f); // table index
					//msg_write(string2("n=%d , %s",nt,( (b[t] & 0x10) > 0 )?"ac":"dc"));
					if ( (b[t] & 0x10) > 0 ) // ac/dc?
						t+=jpg_load_huffman_table(&b[t+1],&jpg_htac[nt])+1;
					else
						t+=jpg_load_huffman_table(&b[t+1],&jpg_htdc[nt])+1;
				}
				b+=seg_len;
			}else if (b[1]==0xda){	// StartOfScan
				//msg_write("SOS");
				b+=4;
				while((b[seg_len]!=0xff)||(b[seg_len+1]==0x00)){
					seg_len++;
				}
				seg_len-=10;
				int nc=*(b++);
				if (nc!=3){
					msg_error("jpg: number of colors != 3 (unsupported)");
					break;
				}
				for (int i=0;i<3;i++){
					int c=*(b++);
					ci.ac[c-1]=b[0]>>4;
					ci.dc[c-1]=*(b++)&0x0f;
				}
				b+=3;
				jpg_decode(b,ci);
				b+=seg_len;
			}else{
				//msg_error("jpg: unknown seg");
				seg_len=b[2]*256+b[3]-2;
				b+=4;
				//msg_write(seg_len);
				b+=seg_len;
			}
			if ((long)b-(long)buf > tt.num){
				msg_error("jpg: end of file");
				break;
			}
		}else{
			msg_error("jpg: broken");
			break;
		}
	}
	/*for (int i=0;i<image.Data.num;i++)
		image.Data[i] = (image.Data[i] | 0xff000000);*/
}

