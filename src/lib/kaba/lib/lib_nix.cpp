#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "../dynamic/exception.h"

#ifdef _X_USE_NIX_
	#include "../../nix/nix.h"
#endif

namespace kaba {


#ifdef _X_USE_NIX_
	#define nix_p(p)		p


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

nix::Texture* __LoadTexture(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return nix::Texture::load(filename));
	return nullptr;
}

nix::Shader* __LoadShader(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return nix::Shader::load(filename));
	return nullptr;
}

nix::Shader* __CreateShader(const string &source) {
	KABA_EXCEPTION_WRAPPER(return nix::Shader::create(source));
	return nullptr;
}

#pragma GCC pop_options



#else
struct FakeTexture {
	int width, height;
	int color_attachments, depth_buffer;
};
	namespace nix{
		typedef int VertexBuffer;
		typedef FakeTexture Texture;
		typedef FakeTexture FrameBuffer;
		typedef int Shader;
		typedef int UniformBuffer;
	};
	#define nix_p(p)		nullptr
#endif



extern const Class *TypeMatrix;
extern const Class *TypeImage;
extern const Class *TypeFloatList;
extern const Class *TypeDynamicArray;
extern const Class *TypePath;
const Class *TypeVertexBuffer;
const Class *TypeVertexBufferP;
const Class *TypeTexture;
const Class *TypeTextureP;
const Class *TypeTexturePList;
const Class *TypeVolumeTexture;
const Class *TypeImageTexture;
const Class *TypeDepthBuffer;
const Class *TypeDepthBufferP;
const Class *TypeFrameBuffer;
const Class *TypeFrameBufferP;
const Class *TypeCubeMap;
const Class *TypeShader;
const Class *TypeShaderP;
const Class *TypeUniformBuffer;

void SIAddPackageNix() {
	add_package("nix");
	
	TypeVertexBuffer	= add_type  ("VertexBuffer", sizeof(nix::VertexBuffer));
	TypeVertexBufferP	= add_type_p(TypeVertexBuffer);
	TypeTexture			= add_type  ("Texture", sizeof(nix::Texture));
	TypeTextureP		= add_type_p(TypeTexture);
	TypeTexturePList	= add_type_l(TypeTextureP);
	TypeImageTexture	= add_type  ("ImageTexture", sizeof(nix::Texture));
	TypeVolumeTexture	= add_type  ("VolumeTexture", sizeof(nix::Texture));
	TypeDepthBuffer		= add_type  ("DepthBuffer", sizeof(nix::Texture));
	TypeDepthBufferP	= add_type_p(TypeDepthBuffer);
	TypeFrameBuffer		= add_type  ("FrameBuffer", sizeof(nix::FrameBuffer));
	TypeFrameBufferP	= add_type_p(TypeFrameBuffer);
	TypeCubeMap			= add_type  ("CubeMap", sizeof(nix::Texture));
	TypeShader			= add_type  ("Shader", sizeof(nix::Shader));
	TypeShaderP			= add_type_p(TypeShader);
	TypeUniformBuffer	= add_type  ("UniformBuffer", sizeof(nix::UniformBuffer));
	
	add_class(TypeVertexBuffer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::VertexBuffer::__init__));
			func_add_param("format", TypeString);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(&nix::VertexBuffer::__delete__));
		class_add_func("update", TypeVoid, nix_p(&nix::VertexBuffer::update));
			func_add_param("index", TypeInt);
			func_add_param("data", TypeDynamicArray);
		class_add_func("create_rect", TypeVoid, nix_p(&nix::VertexBuffer::create_rect));
			func_add_param("dest", TypeRect);
			func_add_param("source", TypeRect);
		class_add_func("count", TypeInt, nix_p(&nix::VertexBuffer::count));


	add_class(TypeTexture);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::Texture::__init__));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(&nix::Texture::__delete__));
		class_add_func("overwrite", TypeVoid, nix_p(&nix::Texture::overwrite));
			func_add_param("image", TypeImage);
		class_add_func("read", TypeVoid, nix_p(&nix::Texture::read));
			func_add_param("image", TypeImage);
		class_add_func("read_float", TypeVoid, nix_p(&nix::Texture::read_float));
			func_add_param("data", TypeFloatList);
		class_add_func("write_float", TypeVoid, nix_p(&nix::Texture::write_float));
			func_add_param("data", TypeFloatList);
		class_add_func("load", TypeTextureP, nix_p(&__LoadTexture), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_element("width", TypeInt, nix_p(&nix::Texture::width));
		class_add_element("height", TypeInt, nix_p(&nix::Texture::height));

	add_class(TypeVolumeTexture);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::VolumeTexture::__init__));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
			func_add_param("format", TypeString);

	add_class(TypeImageTexture);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::ImageTexture::__init__), Flags::OVERRIDE);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);

	add_class(TypeDepthBuffer);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::DepthBuffer::__init__), Flags::OVERRIDE);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);

	add_class(TypeCubeMap);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::CubeMap::__init__));
			func_add_param("size", TypeInt);
			func_add_param("format", TypeString);

	add_class(TypeFrameBuffer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::FrameBuffer::__init__));
			func_add_param("attachments", TypeTexturePList);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(&nix::FrameBuffer::__delete__));
		class_add_const("DEFAULT", TypeFrameBufferP, nix_p(&nix::FrameBuffer::DEFAULT));
		class_add_element("width", TypeInt, nix_p(&nix::FrameBuffer::width));
		class_add_element("height", TypeInt, nix_p(&nix::FrameBuffer::height));
		class_add_element("color_attachments", TypeTexturePList, nix_p(&nix::FrameBuffer::color_attachments));
		class_add_element("depth_buffer", TypeDepthBufferP, nix_p(&nix::FrameBuffer::depth_buffer));

	add_class(TypeShader);
		class_add_func("location", TypeInt, nix_p(&nix::Shader::get_location));
			func_add_param("name", TypeString);
		class_add_func("link_uniform_block", TypeVoid, nix_p(&nix::Shader::link_uniform_block));
			func_add_param("name", TypeString);
			func_add_param("binding", TypeInt);
		class_add_func("set_float", TypeVoid, nix_p(&nix::Shader::set_float));
			func_add_param("loc", TypeInt);
			func_add_param("f", TypeFloat32);
		class_add_func("set_matrix", TypeVoid, nix_p(&nix::Shader::set_matrix));
			func_add_param("loc", TypeInt);
			func_add_param("m", TypeMatrix);
		class_add_func("set_color", TypeVoid, nix_p(&nix::Shader::set_color));
			func_add_param("loc", TypeInt);
			func_add_param("c", TypeColor);
		class_add_func("set_int", TypeVoid, nix_p(&nix::Shader::set_int));
			func_add_param("loc", TypeInt);
			func_add_param("i", TypeInt);
		class_add_func("set", TypeVoid, nix_p(&nix::Shader::set_data));
			func_add_param("loc", TypeInt);
			func_add_param("data", TypePointer);
			func_add_param("size", TypeInt);
		class_add_func("dispatch", TypeVoid, nix_p(&nix::Shader::dispatch));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
		class_add_func("load", TypeShaderP, nix_p(&__LoadShader), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_func("create", TypeShaderP, nix_p(&__CreateShader), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("source", TypeString);
		class_add_const("DEFAULT_3D", TypeShaderP, nix_p(&nix::Shader::default_3d));
		class_add_const("DEFAULT_2D", TypeShaderP, nix_p(&nix::Shader::default_2d));


		add_class(TypeUniformBuffer);
			class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(&nix::UniformBuffer::__init__));
			class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(&nix::UniformBuffer::__delete__));
			class_add_func("update", TypeVoid, nix_p(&nix::UniformBuffer::update));
				func_add_param("data", TypePointer);
				func_add_param("size", TypeInt);
			class_add_func("update", TypeVoid, nix_p(&nix::UniformBuffer::update_array));
				func_add_param("data", TypeDynamicArray);
	
		// drawing
	add_func("init", TypeVoid, nix_p(&nix::init), Flags::STATIC);
	add_func("kill", TypeVoid, nix_p(&nix::kill), Flags::STATIC);
	/*add_func("SetVideoMode", TypeVoid, nix_p(&NixSetVideoMode), Flags::STATIC);
		func_add_param("api", TypeString);
		func_add_param("xres", TypeInt);
		func_add_param("yres", TypeInt);
		func_add_param("fullscreen",TypeBool);*/
#ifdef _X_USE_HUI_
	add_func("start_frame_hui", TypeVoid, nix_p(&nix::start_frame_hui), Flags::STATIC);
	add_func("end_frame_hui", TypeVoid, nix_p(&nix::end_frame_hui), Flags::STATIC);
#else
	add_func("start_frame_hui", TypeVoid, nullptr, Flags::STATIC);
	add_func("end_frame_hui", TypeVoid, nullptr, Flags::STATIC);
#endif
	add_func("bind_frame_buffer", TypeVoid, nix_p(&nix::bind_frame_buffer), Flags::STATIC);
		func_add_param("fb", TypeFrameBuffer);
	add_func("set_viewport", TypeVoid, nix_p(&nix::set_viewport), Flags::STATIC);
		func_add_param("r", TypeRect);
	add_func("reset_to_color", TypeVoid, nix_p(&nix::clear_color), Flags::STATIC);
		func_add_param("c", TypeColor);
	add_func("set_model_matrix", TypeVoid, nix_p(&nix::set_model_matrix), Flags::STATIC);
		func_add_param("m", TypeMatrix);
	add_func("draw_triangles", TypeVoid, nix_p(&nix::draw_triangles), Flags::STATIC);
		func_add_param("vb", TypeVertexBuffer);
	add_func("draw_lines", TypeVoid, nix_p(&nix::draw_lines), Flags::STATIC);
		func_add_param("vb", TypeVertexBuffer);
		func_add_param("contiguous", TypeBool);
	add_func("draw_points", TypeVoid, nix_p(&nix::draw_points), Flags::STATIC);
		func_add_param("vb", TypeVertexBuffer);
	add_func("set_alpha", TypeVoid, nix_p(&nix::set_alpha_mode), Flags::STATIC);
		func_add_param("mode", TypeInt);
	add_func("set_alpha", TypeVoid, nix_p(&nix::set_alpha_sd), Flags::STATIC);
		func_add_param("source", TypeInt);
		func_add_param("dest", TypeInt);
	add_func("set_stencil", TypeVoid, nix_p(&nix::set_stencil), Flags::STATIC);
		func_add_param("mode", TypeInt);
		func_add_param("param", TypeInt);
	add_func("set_projection_perspective", TypeVoid, nix_p(&nix::set_projection_perspective), Flags::STATIC);
	add_func("set_projection_perspective_ext", TypeVoid, nix_p(&nix::set_projection_perspective_ext), Flags::STATIC);
		func_add_param("centerx", TypeFloat32);
		func_add_param("centery", TypeFloat32);
		func_add_param("width_1", TypeFloat32);
		func_add_param("height_1", TypeFloat32);
		func_add_param("zmin", TypeFloat32);
		func_add_param("zmax", TypeFloat32);
	add_func("set_projection_ortho_relative", TypeVoid, nix_p(&nix::set_projection_ortho_relative), Flags::STATIC);
	add_func("set_projection_ortho_pixel", TypeVoid, nix_p(&nix::set_projection_ortho_pixel), Flags::STATIC);
	add_func("set_projection_ortho_ext", TypeVoid, nix_p(&nix::set_projection_ortho_ext), Flags::STATIC);
		func_add_param("centerx", TypeFloat32);
		func_add_param("centery", TypeFloat32);
		func_add_param("map_width", TypeFloat32);
		func_add_param("map_height", TypeFloat32);
		func_add_param("zmin", TypeFloat32);
		func_add_param("zmax", TypeFloat32);
	add_func("set_projection_matrix", TypeVoid, nix_p(&nix::set_projection_matrix), Flags::STATIC);
		func_add_param("m", TypeMatrix);
	add_func("set_view_matrix", TypeVoid, nix_p(&nix::set_view_matrix), Flags::STATIC);
		func_add_param("view_mat", TypeMatrix);
	add_func("set_scissor", TypeVoid, nix_p(&nix::set_scissor), Flags::STATIC);
		func_add_param("r", TypeRect);
	add_func("set_z", TypeVoid, nix_p(&nix::set_z), Flags::STATIC);
		func_add_param("write", TypeBool);
		func_add_param("test", TypeBool);
	add_func("set_cull", TypeVoid, nix_p(&nix::set_cull), Flags::STATIC);
		func_add_param("mode", TypeInt);
	add_func("set_wire", TypeVoid, nix_p(&nix::set_wire), Flags::STATIC);
		func_add_param("enabled", TypeBool);
	add_func("set_material", TypeVoid, nix_p(&nix::set_material), Flags::STATIC);
		func_add_param("albedo", TypeColor);
		func_add_param("roughness", TypeFloat32);
		func_add_param("metal", TypeFloat32);
		func_add_param("emission", TypeColor);
	add_func("set_texture", TypeVoid, nix_p(&nix::set_texture), Flags::STATIC);
		func_add_param("t", TypeTexture);
	add_func("set_textures", TypeVoid, nix_p(&nix::set_textures), Flags::STATIC);
		func_add_param("t", TypeTexturePList);
	add_func("set_shader", TypeVoid, nix_p(&nix::set_shader), Flags::STATIC);
		func_add_param("s", TypeShader);
	add_func("bind_uniform", TypeVoid, nix_p(&nix::bind_uniform), Flags::STATIC);
		func_add_param("buf", TypeUniformBuffer);
		func_add_param("binding", TypeInt);
	add_func("screen_shot_to_image", TypeVoid, nix_p(&nix::screen_shot_to_image), Flags::STATIC);
		func_add_param("im", TypeImage);

	add_ext_var("target", TypeRect, nix_p(&nix::target_rect));
	//add_ext_var("fullscreen", TypeBool, nix_p(&nix::Fullscreen));

	// alpha operations
	add_enum("ALPHA_NONE",             TypeInt, nix_p(nix::AlphaMode::NONE));
	add_enum("ALPHA_ZERO",             TypeInt, nix_p(nix::Alpha::ZERO));
	add_enum("ALPHA_ONE",              TypeInt, nix_p(nix::Alpha::ONE));
	add_enum("ALPHA_COLOR_KEY",        TypeInt, nix_p(nix::AlphaMode::COLOR_KEY_SMOOTH));
	add_enum("ALPHA_COLOR_KEY_HARD",   TypeInt, nix_p(nix::AlphaMode::COLOR_KEY_HARD));
	add_enum("ALPHA_ADD",              TypeInt, nix_p(nix::AlphaMode::ADD));
	add_enum("ALPHA_MATERIAL",         TypeInt, nix_p(nix::AlphaMode::MATERIAL));
	add_enum("ALPHA_SOURCE_COLOR",     TypeInt, nix_p(nix::Alpha::SOURCE_COLOR));
	add_enum("ALPHA_SOURCE_INV_COLOR", TypeInt, nix_p(nix::Alpha::SOURCE_INV_COLOR));
	add_enum("ALPHA_SOURCE_ALPHA",     TypeInt, nix_p(nix::Alpha::SOURCE_ALPHA));
	add_enum("ALPHA_SOURCE_INV_ALPHA", TypeInt, nix_p(nix::Alpha::SOURCE_INV_ALPHA));
	add_enum("ALPHA_DEST_COLOR",       TypeInt, nix_p(nix::Alpha::DEST_COLOR));
	add_enum("ALPHA_DEST_INV_COLOR",   TypeInt, nix_p(nix::Alpha::DEST_INV_COLOR));
	add_enum("ALPHA_DEST_ALPHA",       TypeInt, nix_p(nix::Alpha::DEST_ALPHA));
	add_enum("ALPHA_DEST_INV_ALPHA",   TypeInt, nix_p(nix::Alpha::DEST_INV_ALPHA));
	// stencil operations
	add_enum("STENCIL_NONE",               TypeInt, nix_p(nix::StencilOp::NONE));
	add_enum("STENCIL_INCREASE",           TypeInt, nix_p(nix::StencilOp::INCREASE));
	add_enum("STENCIL_DECREASE",           TypeInt, nix_p(nix::StencilOp::DECREASE));
	add_enum("STENCIL_SET",                TypeInt, nix_p(nix::StencilOp::SET));
	add_enum("STENCIL_MASK_EQUAL",         TypeInt, nix_p(nix::StencilOp::MASK_EQUAL));
	add_enum("STENCIL_MASK_NOT_EQUAL",     TypeInt, nix_p(nix::StencilOp::MASK_NOT_EQUAL));
	add_enum("STENCIL_MASK_LESS",          TypeInt, nix_p(nix::StencilOp::MASK_LESS));
	add_enum("STENCIL_MASK_LESS_EQUAL",    TypeInt, nix_p(nix::StencilOp::MASK_LESS_EQUAL));
	add_enum("STENCIL_MASK_GREATER",       TypeInt, nix_p(nix::StencilOp::MASK_GREATER));
	add_enum("STENCIL_MASK_GREATER_EQUAL", TypeInt, nix_p(nix::StencilOp::MASK_GREATER_EQUAL));
	add_enum("STENCIL_RESET",              TypeInt, nix_p(nix::StencilOp::RESET));
	// fog
	add_enum("FOG_LINEAR", TypeInt, nix_p(nix::FogMode::LINEAR));
	add_enum("FOG_EXP",    TypeInt, nix_p(nix::FogMode::EXP));
	add_enum("FOG_EXP2",   TypeInt, nix_p(nix::FogMode::EXP2));


	add_ext_var("vb_temp", TypeVertexBufferP, nix_p(&nix::vb_temp));
}

};
