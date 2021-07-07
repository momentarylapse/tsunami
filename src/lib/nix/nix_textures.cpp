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
#include "../file/file.h"

// management:
//  Texture.load()
//    -> managed, shared<>, DON'T DELETE
//  new Texture()
//    -> unmanaged, needs manual delete

namespace nix{

shared_array<Texture> textures;
Texture *default_texture = nullptr;
Texture *tex_text = nullptr;
int tex_cube_level = -1;


const unsigned int NO_TEXTURE = 0xffffffff;


//--------------------------------------------------------------------------------------------------
// common stuff
//--------------------------------------------------------------------------------------------------

void init_textures() {

	default_texture = new Texture(16, 16, "rgba:i8");
	Image image;
	image.create(16, 16, White);
	default_texture->overwrite(image);

	tex_text = new Texture;
}

void release_textures() {
	for (Texture *t: weak(textures)) {
		//glBindTexture(GL_TEXTURE_2D, t->texture);
		glDeleteTextures(1, &t->texture);
	}
}

void reincarnate_textures() {
	for (Texture *t: weak(textures)) {
		//glGenTextures(1, &t->texture);
		t->reload();
	}
}


// "sized format"
unsigned int parse_format(const string &_format) {
	if (_format == "r:i8")
		return GL_R8;
	if (_format == "rgb:i8")
		return GL_RGB8;
	if (_format == "rgba:i8")
		return GL_RGBA8;
	if (_format == "r:f32")
		return GL_R32F;
	if (_format == "rgba:f32")
		return GL_RGBA32F;
	if (_format == "r:f16")
		return GL_R16F;
	if (_format == "rgb:f16")
		return GL_RGB16F;
	if (_format == "rgba:f16")
		return GL_RGBA16F;
	if (_format == "d24s8")
		return GL_DEPTH24_STENCIL8;

	msg_error("unknown format: " + _format);
	return GL_RGBA8;
}

Texture::Texture() {
	filename = "-empty-";
	type = Type::NONE;
	internal_format = 0;
	valid = true;
	width = height = nz = samples = 0;
	texture = NO_TEXTURE;
}

int mip_levels(int width, int height) {
	int l = min(width, height);
	for (int n=0; n<20; n++)
		if (l >> n == 0)
			return n;
	return 1;
}


void Texture::_create_2d(int w, int h, const string &_format) {
	width = w;
	height = h;
	type = Type::DEFAULT;
	internal_format = parse_format(_format);

	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureStorage2D(texture, mip_levels(width, height), internal_format, width, height);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(int w, int h, const string &_format) : Texture() {
	msg_write(format("creating texture [%d x %d: %s] ", w, h, _format));
	_create_2d(w, h, _format);
}

VolumeTexture::VolumeTexture(int w, int h, int _nz, const string &_format) : Texture() {
	msg_write(format("creating volume texture [%d x %d x %d: %s] ", w, h, _nz, _format));
	width = w;
	height = h;
	nz = _nz;
	type = Type::VOLUME;
	internal_format = parse_format(_format);

	glCreateTextures(GL_TEXTURE_3D, 1, &texture);
	glTextureStorage3D(texture, 1, internal_format, width, height, nz);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_REPEAT);
}

Texture::~Texture() {
	unload();
	/*foreachi(auto t, textures, i)
		if (t == this) {
			textures.erase(i);
			break;
		}*/
}

void Texture::__init__(int w, int h, const string &f) {
	new(this) Texture(w, h, f);
}

void VolumeTexture::__init__(int nx, int ny, int nz, const string &f) {
	new(this) VolumeTexture(nx, ny, nz, f);
}

void Texture::__delete__() {
	this->~Texture();
}

void TextureClear() {
	msg_error("Texture Clear");
	for (Texture *t: weak(textures))
		msg_write(t->filename.str());
}

Texture *Texture::load(const Path &filename) {
	if (filename.is_empty())
		return nullptr;

	// test existence
	if (!file_exists(filename))
		throw Exception("texture file does not exist: " + filename.str());

	Texture *t = new Texture;
	try {
		t->filename = filename;
		t->reload();
		textures.add(t);
		return t;
	} catch (...) {
		delete t;
	}
	return nullptr;
}

void Texture::reload() {
	msg_write("loading texture: " + filename.str());

	// test the file's existence
	if (!file_exists(filename))
		throw Exception("texture file does not exist!");

	string extension = filename.extension();
	auto image = Image::load(filename);
	overwrite(*image);
	delete image;
}

void Texture::set_options(const string &options) const {
	for (auto &x: options.explode(",")) {
		auto y = x.explode("=");
		if (y.num != 2)
			throw Exception("key=value expected: " + x);
		string key = y[0];
		string value = y[1];
		if (key == "wrap") {
			if (value == "repeat") {
				glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
			} else if (value == "clamp") {
				glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			} else {
				throw Exception("unknown value for key: " + x);
			}
		} else if ((key == "magfilter") or (key == "minfilter")) {
			auto filter = (key == "magfilter") ? GL_TEXTURE_MAG_FILTER : GL_TEXTURE_MIN_FILTER;
			if (value == "linear") {
				glTextureParameteri(texture, filter, GL_LINEAR);
			} else if (value == "nearest") {
				glTextureParameteri(texture, filter, GL_NEAREST);
			} else if (value == "trilinear") {
				glTextureParameteri(texture, filter, GL_LINEAR_MIPMAP_LINEAR);
			} else {
				throw Exception("unknown value for key: " + x);
			}
		} else {
			throw Exception("unknown key: " + key);
		}
	}
}

void Texture::overwrite(const Image &image) {
	if (image.error)
		return;

	if (type == Type::NONE)
		_create_2d(image.width, image.height, "rgba:i8");

	if (width != image.width or height != image.height) {
		//msg_write("texture resize..." + filename.str());
		glDeleteTextures(1, &texture);
		_create_2d(image.width, image.height, "rgba:i8");//image.alpha_used ? "rgba:i8" : "rgb:i8");
	}

	image.set_mode(Image::Mode::RGBA);

	glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);

	if (type == Type::DEFAULT)
		glGenerateTextureMipmap(texture);
}

void Texture::read(Image &image) {
	image.create(width, height, Black);
	glGetTextureSubImage(texture, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image.data.num * sizeof(float), image.data.data);
}

unsigned int _gl_channels_(int channels) {
	if (channels == 1)
		return GL_RED;
	if (channels == 2)
		return GL_RG;
	if (channels == 3)
		return GL_RGB;
	return GL_RGBA;
}

void Texture::read_float(Array<float> &data) {
	int ch = channels();
	data.resize(width * height * ch);
	int format = _gl_channels_(ch);

	glGetTextureSubImage(texture, 0, 0, 0, 0, width, height, 1, format, GL_FLOAT, data.num * sizeof(float), data.data);
}

int Texture::channels() const {
	if ((internal_format == GL_R8) or (internal_format == GL_R32F))
		return 1;
	if ((internal_format == GL_RGB8) or (internal_format == GL_RGB32F))
		return 3;
	return 4;
}

void Texture::write_float(Array<float> &data) {
	int ch = channels();
	int length_expected = width * height * ch;
	if (type == Type::VOLUME)
		length_expected *= nz;
	if (data.num != length_expected) {
		msg_error(format("Texture.write_float: array of length %d given, but %d expected", data.num, length_expected));
		return;
	}

	int format = _gl_channels_(ch);
	if (type == Type::VOLUME) {
		glTextureSubImage3D(texture, 0, 0, 0, 0, width, height, nz, format, GL_FLOAT, &data[0]);
	} else {
		glTextureSubImage2D(texture, 0, 0, 0, width, height, format, GL_FLOAT, &data[0]);
	}
}

void Texture::unload() {
	if (type != Type::NONE) {
		msg_write("unloading texture: " + filename.str());
		glDeleteTextures(1, &texture);
	}
}

void set_texture(Texture *t) {
	//refresh_texture(t);
	if (!t)
		t = default_texture;

	tex_cube_level = -1;
	/*glActiveTexture(GL_TEXTURE0);
	if (t->type == Texture::Type::CUBE){
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, t->texture);
		tex_cube_level = 0;
	} else if (t->type == Texture::Type::IMAGE){
		glBindTexture(GL_TEXTURE_2D, t->texture);
		glBindImageTexture(0, t->texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, t->internal_format);
	} else if (t->type == Texture::Type::VOLUME){
		glBindTexture(GL_TEXTURE_3D, t->texture);
	} else if (t->type == Texture::Type::MULTISAMPLE){
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, t->texture);
	} else {
		glBindTexture(GL_TEXTURE_2D, t->texture);
	}*/

	if (t->type == t->Type::CUBE)
		tex_cube_level = 0;
	glBindTextureUnit(0, t->texture);
}

void set_textures(const Array<Texture*> &textures) {
	/*for (int i=0;i<num_textures;i++)
		if (texture[i] >= 0)
			refresh_texture(texture[i]);*/

	tex_cube_level = -1;
	for (int i=0; i<textures.num; i++) {
		auto t = textures[i];
		if (!t)
			t = default_texture;

		if (t->type == t->Type::CUBE)
			tex_cube_level = i;

		/*glActiveTexture(GL_TEXTURE0+i);
		if (t->type == t->Type::CUBE) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, t->texture);
			tex_cube_level = i;
		} else if (t->type == Texture::Type::IMAGE){
			glBindTexture(GL_TEXTURE_2D, t->texture);
			glBindImageTexture(0, t->texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, t->internal_format);
		} else if (t->type == t->Type::VOLUME) {
			glBindTexture(GL_TEXTURE_3D, t->texture);
		} else if (t->type == t->Type::MULTISAMPLE) {
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, t->texture);
		} else {
			glBindTexture(GL_TEXTURE_2D, t->texture);
		}*/
		glBindTextureUnit(i, t->texture);
	}
}



TextureMultiSample::TextureMultiSample(int w, int h, int _samples, const string &_format) : Texture() {
	msg_write(format("creating texture [%d x %d, %d samples: %s]", w, h, _samples, _format));
	width = w;
	height = h;
	samples = _samples;
	type = Type::MULTISAMPLE;
	filename = "-multisample-";
	internal_format = parse_format(_format);

	glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &texture);
	glTextureStorage2DMultisample(texture, samples, internal_format, width, height, GL_TRUE);
	//glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

ImageTexture::ImageTexture(int _width, int _height, const string &_format) {
	msg_write(format("creating image texture [%d x %d: %s] ", _width, _height, _format));
	filename = "-image-";
	width = _width;
	height = _height;
	type = Type::IMAGE;
	internal_format = parse_format(_format);

	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureStorage2D(texture, 1, internal_format, width, height);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ImageTexture::__init__(int width, int height, const string &format) {
	new(this) ImageTexture(width, height, format);
}


DepthBuffer::DepthBuffer(int _width, int _height, const string &_format) {
	msg_write(format("creating depth texture [%d x %d] ", _width, _height));
	filename = "-depth-";
	width = _width;
	height = _height;
	type = Type::DEPTH;
	internal_format = parse_format(_format);


	// as renderbuffer -> can't sample from it!
	/*glGenRenderbuffers(1, &texture);
	glBindRenderbuffer(GL_RENDERBUFFER, texture);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);*/

	// as texture -> can sample!
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureStorage2D(texture, 1, internal_format, width, height);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void DepthBuffer::__init__(int width, int height, const string &format) {
	new(this) DepthBuffer(width, height, format);
}

RenderBuffer::RenderBuffer(int w, int h, const string &format) : RenderBuffer(w, h, 0, format) {}

RenderBuffer::RenderBuffer(int w, int h, int _samples, const string &format) {
	filename = "-render-buffer-";
	type = Type::RENDERBUFFER;
	width = w;
	height = h;
	samples = _samples;
	internal_format = parse_format(format);

	glGenRenderbuffers(1, &texture);
	glBindRenderbuffer(GL_RENDERBUFFER, texture);
	if (samples > 0)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, internal_format, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
}


CubeMap::CubeMap(int size, const string &_format) {
	msg_write(format("creating cube map [%d x %d x 6]", size, size));
	width = size;
	height = size;
	type = Type::CUBE;
	filename = "-cubemap-";
	internal_format = parse_format(_format);

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(texture, 6, internal_format, width, height);

	if (false) {
		Image im;
		im.create(size, size, Red);
		for (int i=0; i<6; i++)
			overwrite_side(i, im);
	}
}

void CubeMap::__init__(int size, const string &format) {
	new(this) CubeMap(size, format);
}

void CubeMap::fill_side(int side, Texture *source) {
	if (!source)
		return;
	if (source->type == Type::CUBE)
		return;
	Image image;
	image.load(source->filename);
	overwrite_side(side, image);
}

void CubeMap::overwrite_side(int side, const Image &image) {
	//_overwrite(GL_TEXTURE_CUBE_MAP, NixCubeMapTarget[side], image);
	if (image.error)
		return;
	if (width != image.width or height != image.height)
		return;

	image.set_mode(Image::Mode::RGBA);

	glTextureSubImage3D(texture, 0, 0, 0, side, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
}


};
#endif
