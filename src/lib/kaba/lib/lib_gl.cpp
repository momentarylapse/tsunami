#include "../kaba.h"
#include "lib.h"
#include "shared.h"
#include "list.h"
#include "../dynamic/exception.h"

#if __has_include("../../nix/nix.h") && HAS_LIB_GL
	#define KABA_EXPORT_GL
	#include "../../nix/nix.h"
#endif

namespace kaba {


#ifdef KABA_EXPORT_GL
	#define gl_p(p)		p


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

xfer<nix::Texture> __LoadTexture(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return nix::Texture::load(filename));
	return nullptr;
}

xfer<nix::Shader> __ContextLoadShader(nix::Context *ctx, const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return ctx->load_shader(filename));
	return nullptr;
}

xfer<nix::Shader> __ContextCreateShader(nix::Context *ctx, const string &source) {
	KABA_EXCEPTION_WRAPPER(return ctx->create_shader(source));
	return nullptr;
}

#pragma GCC pop_options

class KabaShader : public nix::Shader {
public:
	void __delete__() { this->Shader::~Shader(); }
};


class KabaTexture : public nix::Texture {
public:
	void _cdecl __init__(int width, int height, const string &format) {
		new(this) nix::Texture(width, height, format);
	}
	void _cdecl __delete__() {
		reinterpret_cast<nix::Texture*>(this)->~Texture();
	}

	void _cdecl __init_multi_sample__(int width, int height, int samples, const string &format) {
		new(this) nix::TextureMultiSample(width, height, samples, format);
	}
	void _cdecl __init_volume__(int nx, int ny, int nz, const string &format) {
		new(this) nix::VolumeTexture(nx, ny, nz, format);
	}
	void _cdecl __init_image__(int width, int height, const string &format) {
		new(this) nix::ImageTexture(width, height, format);
	}
	void _cdecl __init_depth__(int width, int height, const string &format) {
		new(this) nix::DepthBuffer(width, height, format);
	}
	void _cdecl __init_cube__(int size, const string &format) {
		new(this) nix::CubeMap(size, format);
	}
};

class KabaFrameBuffer : public nix::FrameBuffer {
public:
	void __init__(const shared_array<nix::Texture> &attachments) {
		new(this) nix::FrameBuffer(attachments);
	}
	void __delete__() {
		reinterpret_cast<nix::FrameBuffer*>(this)->~FrameBuffer();
	}
};


#else
	struct FakeTexture : public Sharable<base::Empty> {
		int width, height;
		int color_attachments, depth_buffer;
	};
	struct FakeContext {
	};
	namespace nix{
		typedef FakeContext Context;
		typedef int VertexBuffer;
		typedef FakeTexture Texture;
		typedef FakeTexture FrameBuffer;
		typedef int Shader;
		typedef int Buffer;
	};
	#define gl_p(p)		nullptr
#endif



extern const Class *TypeMat4;
extern const Class *TypeVec2;
extern const Class *TypeImage;
extern const Class *TypeFloatList;
extern const Class *TypeFloatP;
extern const Class *TypeStringList;
extern const Class *TypeDynamicArray;
extern const Class *TypePath;

void SIAddPackageGl(Context *c) {
	add_package(c, "gl");

	auto TypeContext = add_type("Context", sizeof(nix::Context));
	auto TypeContextXfer = add_type_p_xfer(TypeContext);
	auto TypeVertexBuffer = add_type("VertexBuffer", sizeof(nix::VertexBuffer));
	auto TypeVertexBufferP = add_type_p_raw(TypeVertexBuffer);
	auto TypeVertexBufferRef = add_type_ref(TypeVertexBuffer);
	auto TypeTexture = add_type("Texture", sizeof(nix::Texture));
	auto TypeTextureXfer = add_type_p_xfer(TypeTexture);
	auto TypeTextureP = add_type_p_raw(TypeTexture);
	auto TypeTextureRef = add_type_ref(TypeTexture);
	auto TypeTextureSharedNN = add_type_p_shared_not_null(TypeTexture);
	auto TypeTexturePList = add_type_list(TypeTextureP);
	auto TypeTextureSharedNNList = add_type_list(TypeTextureSharedNN);
	auto TypeImageTexture = add_type("ImageTexture", sizeof(nix::Texture));
	auto TypeVolumeTexture = add_type("VolumeTexture", sizeof(nix::Texture));
	auto TypeDepthBuffer = add_type("DepthBuffer", sizeof(nix::Texture));
	auto TypeDepthBufferP = add_type_p_raw(TypeDepthBuffer);
	auto TypeFrameBuffer = add_type("FrameBuffer", sizeof(nix::FrameBuffer));
	auto TypeFrameBufferRef = add_type_ref(TypeFrameBuffer);
	auto TypeCubeMap = add_type("CubeMap", sizeof(nix::Texture));
	auto TypeShader = add_type("Shader", sizeof(nix::Shader));
	auto TypeShaderRef = add_type_ref(TypeShader);
	auto TypeShaderP = add_type_p_raw(TypeShader);
	auto TypeShaderXfer = add_type_p_xfer(TypeShader);
	auto TypeBuffer = add_type("Buffer", sizeof(nix::Buffer));
	auto TypeBufferP = add_type_p_raw(TypeBuffer);
	auto TypeUniformBuffer = add_type("UniformBuffer", sizeof(nix::Buffer));
	auto TypeShaderStorageBuffer = add_type("ShaderStorageBuffer", sizeof(nix::Buffer));
	auto TypeAlpha = add_type_enum("Alpha");
	auto TypeStencilOp = add_type_enum("StencilOp");
	auto TypeFogMode = add_type_enum("FogMode");
	auto TypeCullMode = add_type_enum("CullMode");
	lib_create_list<nix::Texture*>(TypeTexturePList);

	lib_create_pointer_xfer(TypeTextureXfer);
	lib_create_pointer_xfer(TypeShaderXfer);
	lib_create_pointer_shared<nix::Texture>(TypeTextureSharedNN, TypeTextureXfer);
	

	add_class(TypeContext);
		class_add_element("gl_version", TypeString, gl_p(&nix::Context::gl_version));
		class_add_element("gl_renderer", TypeString, gl_p(&nix::Context::gl_renderer));
		class_add_element("glsl_version", TypeString, gl_p(&nix::Context::glsl_version));
		class_add_element("extensions", TypeStringList, gl_p(&nix::Context::extensions));
		class_add_element("default_framebuffer", TypeFrameBufferRef, gl_p(&nix::Context::default_framebuffer));
		class_add_element("tex_white", TypeTextureRef, gl_p(&nix::Context::tex_white));
		class_add_element("default_shader_2d", TypeShaderRef, gl_p(&nix::Context::default_2d));
		class_add_element("default_shader_3d", TypeShaderRef, gl_p(&nix::Context::default_3d));
		class_add_element("vb_temp", TypeVertexBufferRef, gl_p(&nix::Context::vb_temp));
		class_add_element("vb_temp_i", TypeVertexBufferRef, gl_p(&nix::Context::vb_temp_i));
		class_add_func("load_shader", TypeShaderXfer, gl_p(&__ContextLoadShader), Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_func("create_shader", TypeShaderXfer, gl_p(&__ContextCreateShader), Flags::RAISES_EXCEPTIONS);
			func_add_param("source", TypeString);

	add_class(TypeVertexBuffer);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&nix::VertexBuffer::__init__));
			func_add_param("format", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&nix::VertexBuffer::__delete__));
		class_add_func("update", TypeVoid, gl_p(&nix::VertexBuffer::update));
			func_add_param("vertices", TypeDynamicArray);
		class_add_func("update_index", TypeVoid, gl_p(&nix::VertexBuffer::update_index));
			func_add_param("indices", TypeDynamicArray);
		class_add_func("create_quad", TypeVoid, gl_p(&nix::VertexBuffer::create_quad));
			func_add_param("dest", TypeRect);
			func_add_param("source", TypeRect);
		class_add_func("create_cube", TypeVoid, gl_p(&nix::VertexBuffer::create_cube));
			func_add_param("a", TypeVec3);
			func_add_param("b", TypeVec3);
		class_add_func("count", TypeInt, gl_p(&nix::VertexBuffer::count));


	add_class(TypeTexture);
		class_add_element("width", TypeInt, gl_p(&nix::Texture::width));
		class_add_element("height", TypeInt, gl_p(&nix::Texture::height));
		class_add_element(Identifier::SHARED_COUNT, TypeInt, gl_p(&nix::Texture::_pointer_ref_counter));
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&KabaTexture::__init__));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&KabaTexture::__delete__));
		class_add_func("set_options", TypeVoid, gl_p(&nix::Texture::set_options));
			func_add_param("op", TypeString);
		class_add_func("write", TypeVoid, gl_p(&nix::Texture::write));
			func_add_param("image", TypeImage);
		class_add_func("read", TypeVoid, gl_p(&nix::Texture::read));
			func_add_param("image", TypeImage);
		class_add_func("read_float", TypeVoid, gl_p(&nix::Texture::read_float));
			func_add_param("data", TypeFloatList);
		class_add_func("write_float", TypeVoid, gl_p(&nix::Texture::write_float));
			func_add_param("data", TypeFloatList);
		class_add_func("load", TypeTextureXfer, gl_p(&__LoadTexture), Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);


	lib_create_list<shared<nix::Texture>>(TypeTextureSharedNNList);


	add_class(TypeVolumeTexture);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&KabaTexture::__init_volume__));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
			func_add_param("format", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&KabaTexture::__delete__), Flags::OVERRIDE);

	add_class(TypeImageTexture);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&KabaTexture::__init_image__));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&KabaTexture::__delete__), Flags::OVERRIDE);

	add_class(TypeDepthBuffer);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&KabaTexture::__init_depth__));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&KabaTexture::__delete__), Flags::OVERRIDE);

	add_class(TypeCubeMap);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&KabaTexture::__init_cube__));
			func_add_param("size", TypeInt);
			func_add_param("format", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&KabaTexture::__delete__), Flags::OVERRIDE);

	add_class(TypeFrameBuffer);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&KabaFrameBuffer::__init__));
			func_add_param("attachments", TypeTextureSharedNNList);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&KabaFrameBuffer::__delete__));
		class_add_func("area", TypeRect, gl_p(&nix::FrameBuffer::area));
		class_add_func("clear_color", TypeVoid, gl_p(&nix::FrameBuffer::clear_color));
			func_add_param("index", TypeInt);
			func_add_param("c", TypeColor);
		class_add_func("clear_depth", TypeVoid, gl_p(&nix::FrameBuffer::clear_depth));
			func_add_param("z", TypeFloat32);
		class_add_func("update", TypeVoid, gl_p(&nix::FrameBuffer::update));
			func_add_param("attachments", TypeTextureSharedNNList);
		class_add_func("update", TypeVoid, gl_p(&nix::FrameBuffer::update_x));
			func_add_param("attachments", TypeTextureSharedNNList);
			func_add_param("face", TypeInt);
		class_add_func("read", TypeVoid, gl_p(&nix::FrameBuffer::read));
			func_add_param("image", TypeImage, Flags::OUT);
		//class_add_const("DEFAULT", TypeFrameBufferP, gl_p(&nix::FrameBuffer::DEFAULT));
		class_add_element("width", TypeInt, gl_p(&nix::FrameBuffer::width));
		class_add_element("height", TypeInt, gl_p(&nix::FrameBuffer::height));
		class_add_element("color_attachments", TypeTextureSharedNNList, gl_p(&nix::FrameBuffer::color_attachments));
		class_add_element("depth_buffer", TypeDepthBufferP, gl_p(&nix::FrameBuffer::depth_buffer));
		class_add_element(Identifier::SHARED_COUNT, TypeInt, gl_p(&nix::FrameBuffer::_pointer_ref_counter));

	add_class(TypeShader);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&KabaShader::__delete__));
		class_add_func("location", TypeInt, gl_p(&nix::Shader::get_location));
			func_add_param("name", TypeString);
		class_add_func("link_uniform_block", TypeVoid, gl_p(&nix::Shader::link_uniform_block));
			func_add_param("name", TypeString);
			func_add_param("binding", TypeInt);
		class_add_func("set_float_l", TypeVoid, gl_p(&nix::Shader::set_float_l));
			func_add_param("loc", TypeInt);
			func_add_param("f", TypeFloat32);
		class_add_func("set_matrix_l", TypeVoid, gl_p(&nix::Shader::set_matrix_l));
			func_add_param("loc", TypeInt);
			func_add_param("m", TypeMat4);
		class_add_func("set_color_l", TypeVoid, gl_p(&nix::Shader::set_color_l));
			func_add_param("loc", TypeInt);
			func_add_param("c", TypeColor);
		class_add_func("set_int_l", TypeVoid, gl_p(&nix::Shader::set_int_l));
			func_add_param("loc", TypeInt);
			func_add_param("i", TypeInt);
		class_add_func("set_floats_l", TypeVoid, gl_p(&nix::Shader::set_floats_l));
			func_add_param("loc", TypeInt);
			func_add_param("data", TypeFloatP);
			func_add_param("num", TypeInt);
		class_add_func("set_float", TypeVoid, gl_p(&nix::Shader::set_float));
			func_add_param("name", TypeString);
			func_add_param("f", TypeFloat32);
		class_add_func("set_matrix", TypeVoid, gl_p(&nix::Shader::set_matrix));
			func_add_param("name", TypeString);
			func_add_param("m", TypeMat4);
		class_add_func("set_color", TypeVoid, gl_p(&nix::Shader::set_color));
			func_add_param("name", TypeString);
			func_add_param("c", TypeColor);
		class_add_func("set_int", TypeVoid, gl_p(&nix::Shader::set_int));
			func_add_param("name", TypeString);
			func_add_param("i", TypeInt);
		class_add_func("set_floats", TypeVoid, gl_p(&nix::Shader::set_floats));
			func_add_param("name", TypeString);
			func_add_param("data", TypeFloatP);
			func_add_param("num", TypeInt);
		class_add_func("dispatch", TypeVoid, gl_p(&nix::Shader::dispatch));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
		class_add_element(Identifier::SHARED_COUNT, TypeInt, gl_p(&nix::Shader::_pointer_ref_counter));


	add_class(TypeBuffer);
		class_add_func(Identifier::Func::DELETE, TypeVoid, gl_p(&nix::Buffer::__delete__));
		class_add_func("update", TypeVoid, gl_p(&nix::Buffer::update));
			func_add_param("data", TypeReference);
			func_add_param("size", TypeInt);
		class_add_func("update", TypeVoid, gl_p(&nix::Buffer::update_array));
			func_add_param("data", TypeDynamicArray);
		class_add_func("read", TypeVoid, gl_p(&nix::Buffer::read));
			func_add_param("data", TypeReference);
			func_add_param("size", TypeInt);
		class_add_func("read", TypeVoid, gl_p(&nix::Buffer::read_array));
			func_add_param("data", TypeDynamicArray);

	add_class(TypeUniformBuffer);
		class_derive_from(TypeBuffer);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&nix::UniformBuffer::__init__));

	add_class(TypeShaderStorageBuffer);
		class_derive_from(TypeBuffer);
		class_add_func(Identifier::Func::INIT, TypeVoid, gl_p(&nix::ShaderStorageBuffer::__init__));

		// drawing
	add_func("init", TypeContextXfer, gl_p(&nix::init), Flags::STATIC);
		func_add_param("flags", TypeStringList);
	//add_func("kill", TypeVoid, gl_p(&nix::kill), Flags::STATIC);
#ifdef _X_USE_HUI_
	add_func("start_frame_hui", TypeVoid, gl_p(&nix::start_frame_hui), Flags::STATIC);
		func_add_param("ctx", TypeContext);
	add_func("end_frame_hui", TypeVoid, gl_p(&nix::end_frame_hui), Flags::STATIC);
#else
	add_func("start_frame_hui", TypeVoid, nullptr, Flags::STATIC);
		func_add_param("ctx", TypeContext);
	add_func("end_frame_hui", TypeVoid, nullptr, Flags::STATIC);
#endif
	add_func("bind_frame_buffer", TypeVoid, gl_p(&nix::bind_frame_buffer), Flags::STATIC);
		func_add_param("fb", TypeFrameBuffer); // -> ref
	add_func("set_viewport", TypeVoid, gl_p(&nix::set_viewport), Flags::STATIC);
		func_add_param("r", TypeRect);
	add_func("clear_color", TypeVoid, gl_p(&nix::clear_color), Flags::STATIC);
		func_add_param("c", TypeColor);
	add_func("clear_z", TypeVoid, gl_p(&nix::clear_z), Flags::STATIC);
		func_add_param("z", TypeFloat32);
	add_func("clear", TypeVoid, gl_p(&nix::clear), Flags::STATIC);
		func_add_param("c", TypeColor);
	add_func("set_model_matrix", TypeVoid, gl_p(&nix::set_model_matrix), Flags::STATIC);
		func_add_param("m", TypeMat4);
	add_func("draw_triangles", TypeVoid, gl_p(&nix::draw_triangles), Flags::STATIC);
		func_add_param("vb", TypeVertexBufferP); // -> ref
	add_func("draw_lines", TypeVoid, gl_p(&nix::draw_lines), Flags::STATIC);
		func_add_param("vb", TypeVertexBufferP); // -> ref
		func_add_param("contiguous", TypeBool);
	add_func("draw_points", TypeVoid, gl_p(&nix::draw_points), Flags::STATIC);
		func_add_param("vb", TypeVertexBufferP); // -> ref
	add_func("disable_alpha", TypeVoid, gl_p(&nix::disable_alpha), Flags::STATIC);
	add_func("set_alpha", TypeVoid, gl_p(&nix::set_alpha_sd), Flags::STATIC);
		func_add_param("source", TypeAlpha);
		func_add_param("dest", TypeAlpha);
	add_func("set_stencil", TypeVoid, gl_p(&nix::set_stencil), Flags::STATIC);
		func_add_param("mode", TypeStencilOp);
		func_add_param("param", TypeInt);
	add_func("set_projection_perspective", TypeVoid, gl_p(&nix::set_projection_perspective), Flags::STATIC);
	add_func("set_projection_perspective_ext", TypeVoid, gl_p(&nix::set_projection_perspective_ext), Flags::STATIC);
		func_add_param("center", TypeVec2);
		func_add_param("size_1", TypeVec2);
		func_add_param("zmin", TypeFloat32);
		func_add_param("zmax", TypeFloat32);
	add_func("set_projection_ortho_relative", TypeVoid, gl_p(&nix::set_projection_ortho_relative), Flags::STATIC);
	add_func("set_projection_ortho_pixel", TypeVoid, gl_p(&nix::set_projection_ortho_pixel), Flags::STATIC);
	add_func("set_projection_ortho_ext", TypeVoid, gl_p(&nix::set_projection_ortho_ext), Flags::STATIC);
		func_add_param("center", TypeVec2);
		func_add_param("map_size", TypeVec2);
		func_add_param("zmin", TypeFloat32);
		func_add_param("zmax", TypeFloat32);
	add_func("set_projection_matrix", TypeVoid, gl_p(&nix::set_projection_matrix), Flags::STATIC);
		func_add_param("m", TypeMat4);
	add_func("set_view_matrix", TypeVoid, gl_p(&nix::set_view_matrix), Flags::STATIC);
		func_add_param("view_mat", TypeMat4);
	add_func("set_scissor", TypeVoid, gl_p(&nix::set_scissor), Flags::STATIC);
		func_add_param("r", TypeRect);
	add_func("set_z", TypeVoid, gl_p(&nix::set_z), Flags::STATIC);
		func_add_param("write", TypeBool);
		func_add_param("test", TypeBool);
	add_func("set_cull", TypeVoid, gl_p(&nix::set_cull), Flags::STATIC);
		func_add_param("mode", TypeCullMode);
	add_func("set_wire", TypeVoid, gl_p(&nix::set_wire), Flags::STATIC);
		func_add_param("enabled", TypeBool);
	add_func("set_material", TypeVoid, gl_p(&nix::set_material), Flags::STATIC);
		func_add_param("albedo", TypeColor);
		func_add_param("roughness", TypeFloat32);
		func_add_param("metal", TypeFloat32);
		func_add_param("emission", TypeColor);
	add_func("bind_textures", TypeVoid, gl_p(&nix::set_textures), Flags::STATIC);
		func_add_param("t", TypeTexturePList); // -> ref[]
	add_func("bind_texture", TypeVoid, gl_p(&nix::bind_texture), Flags::STATIC);
		func_add_param("binding", TypeInt);
		func_add_param("t", TypeTextureP); // -> ref
	add_func("set_shader", TypeVoid, gl_p(&nix::set_shader), Flags::STATIC);
		func_add_param("s", TypeShaderP); // -> ref
	add_func("bind_buffer", TypeVoid, gl_p(&nix::bind_buffer), Flags::STATIC);
		func_add_param("binding", TypeInt);
		func_add_param("buf", TypeBufferP); // -> ref
	add_func("bind_image", TypeVoid, gl_p(&nix::bind_image), Flags::STATIC);
		func_add_param("binding", TypeInt);
		func_add_param("t", TypeTextureP); // -> ref
		func_add_param("level", TypeInt);
		func_add_param("layer", TypeInt);
		func_add_param("writable", TypeBool);
	add_func("screen_shot_to_image", TypeVoid, gl_p(&nix::screen_shot_to_image), Flags::STATIC);
		func_add_param("im", TypeImage);

	add_ext_var("target", TypeRect, gl_p(&nix::target_rect));

	// alpha operations
	add_class(TypeAlpha);
		class_add_enum("ZERO",             TypeAlpha, gl_p(nix::Alpha::ZERO));
		class_add_enum("ONE",              TypeAlpha, gl_p(nix::Alpha::ONE));
		class_add_enum("SOURCE_COLOR",     TypeAlpha, gl_p(nix::Alpha::SOURCE_COLOR));
		class_add_enum("SOURCE_INV_COLOR", TypeAlpha, gl_p(nix::Alpha::SOURCE_INV_COLOR));
		class_add_enum("SOURCE_ALPHA",     TypeAlpha, gl_p(nix::Alpha::SOURCE_ALPHA));
		class_add_enum("SOURCE_INV_ALPHA", TypeAlpha, gl_p(nix::Alpha::SOURCE_INV_ALPHA));
		class_add_enum("DEST_COLOR",       TypeAlpha, gl_p(nix::Alpha::DEST_COLOR));
		class_add_enum("DEST_INV_COLOR",   TypeAlpha, gl_p(nix::Alpha::DEST_INV_COLOR));
		class_add_enum("DEST_ALPHA",       TypeAlpha, gl_p(nix::Alpha::DEST_ALPHA));
		class_add_enum("DEST_INV_ALPHA",   TypeAlpha, gl_p(nix::Alpha::DEST_INV_ALPHA));
	// stencil operations
	add_class(TypeStencilOp);
		class_add_enum("NONE",               TypeStencilOp, gl_p(nix::StencilOp::NONE));
		class_add_enum("INCREASE",           TypeStencilOp, gl_p(nix::StencilOp::INCREASE));
		class_add_enum("DECREASE",           TypeStencilOp, gl_p(nix::StencilOp::DECREASE));
		class_add_enum("SET",                TypeStencilOp, gl_p(nix::StencilOp::SET));
		class_add_enum("MASK_EQUAL",         TypeStencilOp, gl_p(nix::StencilOp::MASK_EQUAL));
		class_add_enum("MASK_NOT_EQUAL",     TypeStencilOp, gl_p(nix::StencilOp::MASK_NOT_EQUAL));
		class_add_enum("MASK_LESS",          TypeStencilOp, gl_p(nix::StencilOp::MASK_LESS));
		class_add_enum("MASK_LESS_EQUAL",    TypeStencilOp, gl_p(nix::StencilOp::MASK_LESS_EQUAL));
		class_add_enum("MASK_GREATER",       TypeStencilOp, gl_p(nix::StencilOp::MASK_GREATER));
		class_add_enum("MASK_GREATER_EQUAL", TypeStencilOp, gl_p(nix::StencilOp::MASK_GREATER_EQUAL));
		class_add_enum("RESET",              TypeStencilOp, gl_p(nix::StencilOp::RESET));
	// fog
	add_class(TypeFogMode);
		class_add_enum("LINEAR", TypeFogMode, gl_p(nix::FogMode::LINEAR));
		class_add_enum("EXP",    TypeFogMode, gl_p(nix::FogMode::EXP));
		class_add_enum("EXP2",   TypeFogMode, gl_p(nix::FogMode::EXP2));
	// culling
	add_class(TypeCullMode);
		class_add_enum("NONE", TypeCullMode, gl_p(nix::CullMode::NONE));
		class_add_enum("BACK",   TypeCullMode, gl_p(nix::CullMode::BACK));
		class_add_enum("FRONT",  TypeCullMode, gl_p(nix::CullMode::FRONT));



//	add_ext_var("vb_temp", TypeVertexBufferRef, gl_p(&nix::vb_temp));
}

};
