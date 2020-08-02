/*----------------------------------------------------------------------------*\
| Nix textures                                                                 |
| -> texture loading and handling                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.11.09 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"
#include "../image/image.h"

namespace nix{

Path texture_dir;

Array<Texture*> textures;
Texture *default_texture = NULL;
Texture *tex_text = NULL;
int tex_cube_level = -1;


//void SetDefaultShaderData(int num_textures, const vector &cam_pos);



//--------------------------------------------------------------------------------------------------
// avi files
//--------------------------------------------------------------------------------------------------

#ifdef NIX_ALLOW_VIDEO_TEXTURE


struct s_avi_info
{
	AVISTREAMINFO		psi;										// Pointer To A Structure Containing Stream Info
	PAVISTREAM			pavi;										// Handle To An Open Stream
	PGETFRAME			pgf;										// Pointer To A GetFrame Object
	BITMAPINFOHEADER	bmih;										// Header Information For DrawDibDraw Decoding
	long				lastframe;									// Last Frame Of The Stream
	int					width;										// Video Width
	int					height;										// Video Height
	char				*pdata;										// Pointer To Texture Data
	unsigned char*		data;										// Pointer To Our Resized Image
	HBITMAP hBitmap;												// Handle To A Device Dependant Bitmap
	float time,fps;
	int ActualFrame;
	HDRAWDIB hdd;												// Handle For Our Dib
	HDC hdc;										// Creates A Compatible Device Context
}*avi_info[NIX_MAX_TEXTURES];


static void avi_flip(void* buffer,int w,int h)
{
	unsigned char *b = (BYTE *)buffer;
	char temp;
    for (int x=0;x<w;x++)
    	for (int y=0;y<h/2;y++){
    		temp=b[(x+(h-y-1)*w)*3+2];
    		b[(x+(h-y-1)*w)*3+2]=b[(x+y*w)*3  ];
    		b[(x+y*w)*3  ]=temp;

    		temp=b[(x+(h-y-1)*w)*3+1];
    		b[(x+(h-y-1)*w)*3+1]=b[(x+y*w)*3+1];
    		b[(x+y*w)*3+1]=temp;

    		temp=b[(x+(h-y-1)*w)*3  ];
    		b[(x+(h-y-1)*w)*3  ]=b[(x+y*w)*3+2];
    		b[(x+y*w)*3+2]=temp;
    	}
}

static int GetBestVideoSize(int s)
{
	return NixMaxVideoTextureSize;
}

void avi_grab_frame(int texture,int frame)									// Grabs A Frame From The Stream
{
	if (texture<0)
		return;
	if (!avi_info[texture])
		return;
	if (avi_info[texture]->ActualFrame==frame)
		return;
	int w=NixTextureWidth[texture];
	int h=NixTextureHeight[texture];
	msg_write(w);
	msg_write(h);
	avi_info[texture]->ActualFrame=frame;
	LPBITMAPINFOHEADER lpbi;									// Holds The Bitmap Header Information
	lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(avi_info[texture]->pgf, frame);	// Grab Data From The AVI Stream
	avi_info[texture]->pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);	// Pointer To Data Returned By AVIStreamGetFrame

	// Convert Data To Requested Bitmap Format
	DrawDibDraw (avi_info[texture]->hdd, avi_info[texture]->hdc, 0, 0, w, h, lpbi, avi_info[texture]->pdata, 0, 0, avi_info[texture]->width, avi_info[texture]->height, 0);
	

	//avi_flip(avi_info[texture]->data,w,h);	// Swap The Red And Blue Bytes (GL Compatability)

	// Update The Texture
	glBindTexture(GL_TEXTURE_2D,OGLTexture[texture]);
	glEnable(GL_TEXTURE_2D);
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, avi_info[texture]->data);
	//gluBuild2DMipmaps(GL_TEXTURE_2D,3,w,h,GL_RGB,GL_UNSIGNED_BYTE,AVIdata);
}

bool avi_open(int texture,LPCSTR szFile)
{
	avi_info[texture]->hdc = CreateCompatibleDC(0);
	avi_info[texture]->hdd = DrawDibOpen();

	// Opens The AVI Stream
	if (AVIStreamOpenFromFile(&avi_info[texture]->pavi, sys_str_f(szFile), streamtypeVIDEO, 0, OF_READ, NULL) !=0){
		msg_error("Failed To Open The AVI Stream");
		return false;
	}

	AVIStreamInfo(avi_info[texture]->pavi, &avi_info[texture]->psi, sizeof(avi_info[texture]->psi));						// Reads Information About The Stream Into psi
	avi_info[texture]->width=avi_info[texture]->psi.rcFrame.right-avi_info[texture]->psi.rcFrame.left;					// Width Is Right Side Of Frame Minus Left
	avi_info[texture]->height=avi_info[texture]->psi.rcFrame.bottom-avi_info[texture]->psi.rcFrame.top;

	avi_info[texture]->lastframe=AVIStreamLength(avi_info[texture]->pavi);
	avi_info[texture]->fps=float(avi_info[texture]->lastframe)/float(AVIStreamSampleToTime(avi_info[texture]->pavi,avi_info[texture]->lastframe)/1000.0f);

	avi_info[texture]->bmih.biSize = sizeof (BITMAPINFOHEADER);					// Size Of The BitmapInfoHeader
	avi_info[texture]->bmih.biPlanes = 1;											// Bitplanes
	avi_info[texture]->bmih.biBitCount = 24;										// Bits Format We Want (24 Bit, 3 Bytes)
//	avi_info[texture]->bmih.biWidth = AVI_TEXTURE_WIDTH;											// Width We Want (256 Pixels)
//	avi_info[texture]->bmih.biHeight = AVI_TEXTURE_HEIGHT;										// Height We Want (256 Pixels)
	avi_info[texture]->bmih.biCompression = BI_RGB;								// Requested Mode = RGB

	avi_info[texture]->hBitmap = CreateDIBSection (avi_info[texture]->hdc, (BITMAPINFO*)(&avi_info[texture]->bmih), DIB_RGB_COLORS, (void**)(&avi_info[texture]->data), NULL, 0);
	SelectObject (avi_info[texture]->hdc, avi_info[texture]->hBitmap);								// Select hBitmap Into Our Device Context (hdc)

	avi_info[texture]->pgf=AVIStreamGetFrameOpen(avi_info[texture]->pavi, NULL);						// Create The PGETFRAME	Using Our Request Mode
	if (avi_info[texture]->pgf==NULL){
		msg_error("Failed To Open The AVI Frame");
		return false;
	}

	NixTextureWidth[texture]=GetBestVideoSize(avi_info[texture]->width);
	NixTextureHeight[texture]=GetBestVideoSize(avi_info[texture]->height);
	msg_write(NixTextureWidth[texture]);
	msg_write(NixTextureHeight[texture]);

	avi_info[texture]->time=0;
	avi_info[texture]->ActualFrame=1;
	avi_grab_frame(texture,1);
	return true;
}

void avi_close(int texture)
{
	if (!avi_info[texture])
		return;
	DeleteObject(avi_info[texture]->hBitmap);										// Delete The Device Dependant Bitmap Object
	DrawDibClose(avi_info[texture]->hdd);											// Closes The DrawDib Device Context
	AVIStreamGetFrameClose(avi_info[texture]->pgf);								// Deallocates The GetFrame Resources
	AVIStreamRelease(avi_info[texture]->pavi);										// Release The Stream
	//AVIFileExit();												// Release The File
}

#endif



//#endif

//--------------------------------------------------------------------------------------------------
// common stuff
//--------------------------------------------------------------------------------------------------

void init_textures() {
	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		// allow AVI textures
		AVIFileInit();
	#endif


	default_texture = new Texture;
	Image image;
	image.create(16, 16, White);
	default_texture->overwrite(image);

	tex_text = new Texture;
}

void ReleaseTextures() {
	for (Texture *t: textures) {
		glBindTexture(GL_TEXTURE_2D, t->texture);
		glDeleteTextures(1, &t->texture);
	}
}

void ReincarnateTextures() {
	for (Texture *t: textures) {
		glGenTextures(1, &t->texture);
		t->reload();
	}
}


struct FormatData {
	unsigned int internal_format;
	unsigned int components, x;
};

FormatData parse_format(const string &_format) {
	if (_format == "r:i8")
		return {GL_R8, GL_RED, GL_UNSIGNED_BYTE};
	if (_format == "rgb:i8")
		return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE};
	if (_format == "rgba:i8")
		return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
	if (_format == "r:f32")
		return {GL_R32F, GL_RED, GL_FLOAT};
	if (_format == "rgba:f32")
		return {GL_RGBA32F, GL_RGBA, GL_FLOAT};
	if (_format == "r:f16")
		return {GL_R16F, GL_RED, GL_HALF_FLOAT};
	if (_format == "rgba:f16")
		return {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT};

	msg_error("unknown format: " + _format);
	return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
}

Texture::Texture() {
	filename = "-empty-";
	type = Type::DEFAULT;
	internal_format = 0;
	valid = true;
#ifdef NIX_ALLOW_VIDEO_TEXTURE
	avi_info = NULL;
#endif
	glGenTextures(1, &texture);
	width = height = 0;

	textures.add(this);
}


Texture::Texture(int w, int h, const string &_format) : Texture() {
	msg_write(format("creating texture [%d x %d] ", w, h) + _format);
	width = w;
	height = h;

	glBindTexture(GL_TEXTURE_2D, texture);
	auto d = parse_format(_format);
	internal_format = d.internal_format;
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, d.components, d.x, 0);
	TestGLError("Texture: glTexImage2D");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	TestGLError("Texture: parameter");
}

Texture::~Texture() {
	unload();
}

void Texture::__init__(int w, int h, const string &f) {
	new(this) Texture(w, h, f);
}

void Texture::__delete__() {
	this->~Texture();
}

Texture *LoadTexture(const Path &filename) {
	if (filename.is_empty())
		return nullptr;
	for (Texture *t: textures)
		if (filename == t->filename)
			return t->valid ? t : nullptr;

	// test existence
	if (!file_exists(texture_dir << filename)) {
		msg_error("texture file does not exist: " + filename.str());
		return nullptr;
	}

	Texture *t = new Texture;
	t->filename = filename;
	t->reload();
	return t;
}

void Texture::reload() {
	msg_write("loading texture: " + filename.str());

	Path _filename = texture_dir << filename;

	// test the file's existence
	if (!file_exists(_filename)) {
		msg_error("texture file does not exist!");
		return;
	}

	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		avi_info[texture]=NULL;
	#endif

	string extension = filename.extension();

// AVI
	if (extension == "avi"){
		//msg_write("avi");
		#ifdef NIX_ALLOW_VIDEO_TEXTURE
			avi_info[texture]=new s_avi_info;

			glBindTexture(GL_TEXTURE_2D, texture);
			glEnable(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			if (!avi_open(texture,SysFileName(t->Filename))){
				avi_info[texture]=NULL;
				return;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->Width, t->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, avi_info[texture]->data);
		#else
			msg_error("Support for video textures is not activated!!!");
			msg_write("-> un-comment the NIX_ALLOW_VIDEO_TEXTURE definition in the source file \"00_config.h\" and recompile the program");
			return;
		#endif
	}else{
		auto image = Image::load(_filename);
		overwrite(*image);
		delete image;
	}
}

void OverwriteTexture__(Texture *t, int target, int subtarget, const Image &image) {
	if (!t)
		return;

	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		avi_info[texture]=NULL;
	#endif

	image.set_mode(Image::Mode::RGBA);

	if (t->type == t->Type::CUBE)
		target = GL_TEXTURE_CUBE_MAP;

	if (!image.error){
		//glEnable(target);
		glBindTexture(target, t->texture);
		TestGLError("OverwriteTexture a");
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		TestGLError("OverwriteTexture b");
		if (t->type == t->Type::CUBE){
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}else if (t->type == t->Type::DYNAMIC){
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}else{
			//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		TestGLError("OverwriteTexture c");
#ifdef GL_GENERATE_MIPMAP
		//if (image.alpha_used) {
			t->internal_format = GL_RGBA8;
			glTexImage2D(subtarget, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
		//} else {
		//	t->internal_format = GL_RGB8;
		//	glTexImage2D(subtarget, 0, GL_RGB8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
		//}
		TestGLError("OverwriteTexture d");
		if (t->type == t->Type::DEFAULT)
			glGenerateMipmap(GL_TEXTURE_2D);
		TestGLError("OverwriteTexture e");
#else
		if (image.alpha_used)
			gluBuild2DMipmaps(subtarget,4,image.width,image.height,GL_RGBA,GL_UNSIGNED_BYTE, image.data.data);
		else
			gluBuild2DMipmaps(subtarget,3,image.width,image.height,GL_RGBA,GL_UNSIGNED_BYTE, image.data.data);
#endif
					
	//msg_todo("32 bit textures for OpenGL");
		//gluBuild2DMipmaps(subtarget,4,NixImage.width,NixImage.height,GL_RGBA,GL_UNSIGNED_BYTE,NixImage.data);
		//glTexImage2D(subtarget,0,GL_RGBA8,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,NixImage.data);
		//glTexImage2D(subtarget,0,4,256,256,0,4,GL_UNSIGNED_BYTE,NixImage.data);

		t->width = image.width;
		t->height = image.height;
	}
}

void Texture::overwrite(const Image &image) {
	OverwriteTexture__(this, GL_TEXTURE_2D, GL_TEXTURE_2D, image);
}

void Texture::read(Image &image) {
	SetTexture(this);
	image.create(width, height, Black);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
}

void Texture::read_float(Array<float> &data) {
	SetTexture(this);
	if ((internal_format == GL_R8) or (internal_format == GL_R32F)) {
		data.resize(width * height);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data.data); // 1 channel
	} else {
		data.resize(width * height * 4);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data); // 4 channels
	}
}

void Texture::write_float(Array<float> &data, int nx, int ny, int nz) {
	msg_todo("Texture.write_float");
	SetTexture(this);
	if ((internal_format == GL_R8) or (internal_format == GL_R32F)) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nx, ny, nz, GL_RGBA, GL_FLOAT, &data[0]);
		//glSetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data.data); // 1 channel
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nx, ny, nz, GL_RGBA, GL_FLOAT, &data[0]);
		//glSetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data); // 4 channels
	}
}

void Texture::unload() {
	msg_write("unloading Texture: " + filename.str());
	glBindTexture(GL_TEXTURE_2D, texture);
	glDeleteTextures(1, (unsigned int*)&texture);
}

void SetTexture(Texture *t)
{
	//refresh_texture(t);
	if (!t)
		t = default_texture;

	tex_cube_level = -1;
	glActiveTexture(GL_TEXTURE0);
	TestGLError("SetTex .a");
	if (t->type == Texture::Type::CUBE){
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, t->texture);
		tex_cube_level = 0;
		TestGLError("SetTex b cm");
	} else if (t->type == Texture::Type::IMAGE){
		glBindTexture(GL_TEXTURE_2D, t->texture);
		glBindImageTexture(0, t->texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, t->internal_format);
		TestGLError("SetTex b");
	} else {
		glBindTexture(GL_TEXTURE_2D, t->texture);
		TestGLError("SetTex b");
	}
}

void SetTextures(const Array<Texture*> &textures)
{
	/*for (int i=0;i<num_textures;i++)
		if (texture[i] >= 0)
			refresh_texture(texture[i]);*/

	tex_cube_level = -1;
	for (int i=0; i<textures.num; i++){
		Texture *t = textures[i];
		if (!t)
			t = default_texture;
		glActiveTexture(GL_TEXTURE0+i);
		if (t->type == t->Type::CUBE){
			glBindTexture(GL_TEXTURE_CUBE_MAP, t->texture);
			tex_cube_level = i;
		}else{
			glBindTexture(GL_TEXTURE_2D, t->texture);
		}
		//TestGLError("SetTex"+i2s(i));
	}
	TestGLError("SetTextures");
}

void SetTextureVideoFrame(int texture,int frame)
{
#ifdef NIX_ALLOW_VIDEO_TEXTURE
	if (texture<0)
		return;
	if (!avi_info[texture])
		return;
	if (frame>avi_info[texture]->lastframe)
		frame=0;
	avi_grab_frame(texture,frame);
	avi_info[texture]->time=float(frame)/avi_info[texture]->fps;
#endif
}

void TextureVideoMove(int texture,float elapsed)
{
#ifdef NIX_ALLOW_VIDEO_TEXTURE
	if (texture<0)
		return;
	if (!avi_info[texture])
		return;
	msg_write("<NixTextureVideoMove>");
	avi_info[texture]->time+=elapsed;
	if (avi_info[texture]->time*avi_info[texture]->fps>avi_info[texture]->lastframe)
		avi_info[texture]->time=0;
	avi_grab_frame(texture,int(avi_info[texture]->time*avi_info[texture]->fps));
	msg_write("</NixTextureVideoMove>");
#endif
}


ImageTexture::ImageTexture(int _width, int _height, const string &_format) {
	msg_write(format("creating image texture [%d x %d] ", _width, _height) + _format);
	filename = "-image-";
	width = _width;
	height = _height;
	type = Type::IMAGE;

	glBindTexture(GL_TEXTURE_2D, texture);
	auto d = parse_format(_format);
	internal_format = d.internal_format;
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, d.components, d.x, 0);
	TestGLError("ImageTexture: glTexImage2D");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	TestGLError("ImageTexture: aaaa");
}

void ImageTexture::__init__(int width, int height, const string &format) {
	new(this) ImageTexture(width, height, format);
}

DepthBuffer::DepthBuffer(int _width, int _height) {
	msg_write(format("creating depth texture [%d x %d] ", _width, _height));
	filename = "-depth-";
	width = _width;
	height = _height;
	type = Type::DEPTH;
	internal_format = GL_DEPTH_COMPONENT;


	// as renderbuffer -> can't sample from it!
	/*glGenRenderbuffers(1, &texture);
	TestGLError("FrameBuffer: glGenRenderbuffers");
	glBindRenderbuffer(GL_RENDERBUFFER, texture);
	TestGLError("FrameBuffer: glBindRenderbuffer");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	TestGLError("FrameBuffer: glRenderbufferStorage");*/

	// as texture -> can sample!
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	TestGLError("DepthTexture: aaaa");
}

void DepthBuffer::__init__(int width, int height) {
	new(this) DepthBuffer(width, height);
}



static int NixCubeMapTarget[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

CubeMap::CubeMap(int size) {
	msg_write(format("creating cube map [%d x %d x 6]", size, size));
	width = size;
	height = size;
	type = Type::CUBE;
	filename = "-cubemap-";
}

void CubeMap::__init__(int size) {
	new(this) CubeMap(size);
}

void CubeMap::fill_side(int side, Texture *source) {
	if (!source)
		return;
	if (source->type == Type::CUBE)
		return;
	Image image;
	image.load(texture_dir << source->filename);
	overwrite_side(side, image);
}

void CubeMap::overwrite_side(int side, const Image &image) {
	OverwriteTexture__(this, GL_TEXTURE_CUBE_MAP, NixCubeMapTarget[side], image);
}

#ifdef NIX_API_DIRECTX9
void SetCubeMatrix(vector pos,vector ang)
{
	// TODO
	if (NixApi==NIX_API_DIRECTX9){
		matrix t,r;
		MatrixTranslation(t,-pos);
		MatrixRotationView(r,ang);
		MatrixMultiply(NixViewMatrix,r,t);
		lpDevice->SetTransform(D3DTS_VIEW,(D3DXMATRIX*)&NixViewMatrix);
		lpDevice->Clear(0,NULL,D3DCLEAR_ZBUFFER,0,1.0f,0);
	}
}
void Texture::render_to_cube_map(vector &pos,callback_function *render_func,int mask)
{
	if (mask<1)	return;
	// TODO
	if (NixApi==NIX_API_DIRECTX9){
		HRESULT hr;
		matrix vm=NixViewMatrix;

		hr=DXRenderToEnvMap[cube_map]->BeginCube(DXCubeMap[cube_map]);
		if (hr!=D3D_OK){
			msg_error(string("DXRenderToEnvMap: ",DXErrorMsg(hr)));
			return;
		}
		D3DXMatrixPerspectiveFovLH((D3DXMATRIX*)&NixProjectionMatrix,pi/2,1,NixMinDepth,NixMaxDepth);
		lpDevice->SetTransform(D3DTS_PROJECTION,(D3DXMATRIX*)&NixProjectionMatrix);
		MatrixInverse(NixInvProjectionMatrix,NixProjectionMatrix);
		NixTargetWidth=NixTargetHeight=CubeMapSize[cube_map];
		DXViewPort.X=DXViewPort.Y=0;
		DXViewPort.Width=DXViewPort.Height=CubeMapSize[cube_map];
		lpDevice->SetViewport(&DXViewPort);

		if (mask&1){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_POSITIVE_X,0);
			SetCubeMatrix(pos,vector(0,pi/2,0));
			render_func();
		}
		if (mask&2){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_NEGATIVE_X,0);
			SetCubeMatrix(pos,vector(0,-pi/2,0));
			render_func();
		}
		if (mask&4){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_POSITIVE_Y,0);
			SetCubeMatrix(pos,vector(-pi/2,0,0));
			render_func();
		}
		if (mask&8){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_NEGATIVE_Y,0);
			SetCubeMatrix(pos,vector(pi/2,0,0));
			render_func();
		}
		if (mask&16){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_POSITIVE_Z,0);
			SetCubeMatrix(pos,v0);
			render_func();
		}
		if (mask&32){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_NEGATIVE_Z,0);
			SetCubeMatrix(pos,vector(0,pi,0));
			render_func();
		}
		DXRenderToEnvMap[cube_map]->End(0);

		/*lpDevice->BeginScene();
		SetCubeMatrix(pos,vector(0,pi,0));
		render_func();
		//lpDevice->EndScene();
		End();*/

		NixViewMatrix=vm;
		NixSetView(true,NixViewMatrix);
	}
#ifdef NIX_API_OPENGL
	if (Api==NIX_API_OPENGL){
		msg_todo("RenderToCubeMap for OpenGL");
	}
#endif
}
#endif

};
#endif
