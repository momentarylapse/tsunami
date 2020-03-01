#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"
#include "exception.h"

#ifdef _X_USE_NIX_
	#include "../../nix/nix.h"
#endif

namespace Kaba{


#ifdef _X_USE_NIX_
	#define nix_p(p)		(void*)p


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

nix::Texture* __LoadTexture(const string &filename) {
	KABA_EXCEPTION_WRAPPER(return nix::LoadTexture(filename));
	return nullptr;
}

#pragma GCC pop_options



#else
struct FakeTexture {
	int width, height;
}
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
const Class *TypeVertexBuffer;
const Class *TypeVertexBufferP;
const Class *TypeTexture;
const Class *TypeTextureP;
const Class *TypeTexturePList;
const Class *TypeImageTexture;
const Class *TypeDepthBuffer;
const Class *TypeFrameBuffer;
const Class *TypeFrameBufferP;
const Class *TypeCubeMap;
const Class *TypeShader;
const Class *TypeShaderP;
const Class *TypeUniformBuffer;

void SIAddPackageNix()
{
	add_package("nix", false);
	
	TypeVertexBuffer	= add_type  ("VertexBuffer", sizeof(nix::VertexBuffer));
	TypeVertexBufferP	= add_type_p(TypeVertexBuffer);
	TypeTexture			= add_type  ("Texture", sizeof(nix::Texture));
	TypeTextureP		= add_type_p(TypeTexture);
	TypeTexturePList	= add_type_l(TypeTextureP);
	TypeImageTexture	= add_type  ("ImageTexture", sizeof(nix::Texture));
	TypeDepthBuffer		= add_type  ("DepthBuffer", sizeof(nix::Texture));
	TypeFrameBuffer		= add_type  ("FrameBuffer", sizeof(nix::FrameBuffer));
	TypeFrameBufferP	= add_type_p(TypeFrameBuffer);
	TypeCubeMap			= add_type  ("CubeMap", sizeof(nix::Texture));
	TypeShader			= add_type  ("Shader", sizeof(nix::Shader));
	TypeShaderP			= add_type_p(TypeShader);
	TypeUniformBuffer	= add_type  ("UniformBuffer", sizeof(nix::UniformBuffer));
	
	add_class(TypeVertexBuffer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::VertexBuffer::__init__)));
			func_add_param("format", TypeString);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::VertexBuffer::__delete__)));
		class_add_func("update", TypeVoid, nix_p(mf(&nix::VertexBuffer::update)));
			func_add_param("index", TypeInt);
			func_add_param("data", TypeDynamicArray);
		class_add_func("count", TypeInt, nix_p(mf(&nix::VertexBuffer::count)));


	add_class(TypeTexture);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::Texture::__init__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::Texture::__delete__)));
		class_add_func("overwrite", TypeVoid, nix_p(mf(&nix::Texture::overwrite)));
			func_add_param("image", TypeImage);
		class_add_func("read", TypeVoid, nix_p(mf(&nix::Texture::read)));
			func_add_param("image", TypeImage);
		class_add_func("read_float", TypeVoid, nix_p(mf(&nix::Texture::read_float)));
			func_add_param("data", TypeFloatList);
		class_add_func("write_float", TypeVoid, nix_p(mf(&nix::Texture::write_float)));
			func_add_param("data", TypeFloatList);
		class_add_func("load", TypeTextureP, nix_p(&__LoadTexture), FLAG_STATIC);
			func_add_param("filename", TypeString);
		class_add_elementx("width", TypeInt, &nix::Texture::width);
		class_add_elementx("height", TypeInt, &nix::Texture::height);

	add_class(TypeImageTexture);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::ImageTexture::__init__)), FLAG_OVERRIDE);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);

	add_class(TypeDepthBuffer);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::DepthBuffer::__init__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);

	add_class(TypeCubeMap);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::CubeMap::__init__)));
			func_add_param("size", TypeInt);

	add_class(TypeFrameBuffer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::FrameBuffer::__init__)));
			func_add_param("attachments", TypeTexturePList);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::FrameBuffer::__delete__)));
		class_add_const("DEFAULT", TypeFrameBufferP, nix_p(&nix::FrameBuffer::DEFAULT));
		class_add_elementx("width", TypeInt, &nix::FrameBuffer::width);
		class_add_elementx("height", TypeInt, &nix::FrameBuffer::height);

	add_class(TypeShader);
		class_add_func("unref", TypeVoid, nix_p(mf(&nix::Shader::unref)));
		class_add_func("location", TypeInt, nix_p(mf(&nix::Shader::get_location)));
			func_add_param("name", TypeString);
		class_add_func("link_uniform_block", TypeVoid, nix_p(mf(&nix::Shader::link_uniform_block)));
			func_add_param("name", TypeString);
			func_add_param("binding", TypeInt);
		class_add_func("set_float", TypeVoid, nix_p(mf(&nix::Shader::set_float)));
			func_add_param("loc", TypeInt);
			func_add_param("f", TypeFloat32);
		class_add_func("set_matrix", TypeVoid, nix_p(mf(&nix::Shader::set_matrix)));
			func_add_param("loc", TypeInt);
			func_add_param("m", TypeMatrix);
		class_add_func("set_color", TypeVoid, nix_p(mf(&nix::Shader::set_color)));
			func_add_param("loc", TypeInt);
			func_add_param("c", TypeColor);
		class_add_func("set_int", TypeVoid, nix_p(mf(&nix::Shader::set_int)));
			func_add_param("loc", TypeInt);
			func_add_param("i", TypeInt);
		class_add_func("set", TypeVoid, nix_p(mf(&nix::Shader::set_data)));
			func_add_param("loc", TypeInt);
			func_add_param("data", TypePointer);
			func_add_param("size", TypeInt);
		/*class_add_func("get", TypeVoid, nix_p(mf(&nix::Shader::get_data)));
			func_add_param("loc", TypeInt);
			func_add_param("data", TypePointer);
			func_add_param("size", TypeInt);*/
		class_add_func("dispatch", TypeVoid, nix_p(mf(&nix::Shader::dispatch)));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
		class_add_func("load", TypeShaderP, nix_p(&nix::Shader::load), FLAG_STATIC);
			func_add_param("filename", TypeString);
		class_add_func("create", TypeShaderP, nix_p(&nix::Shader::create), FLAG_STATIC);
			func_add_param("source", TypeString);
		class_add_const("DEFAULT_3D", TypeShaderP, nix_p(&nix::default_shader_3d));
		class_add_const("DEFAULT_2D", TypeShaderP, nix_p(&nix::default_shader_2d));


		add_class(TypeUniformBuffer);
			class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::UniformBuffer::__init__)));
			class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::UniformBuffer::__delete__)));
			class_add_func("update", TypeVoid, nix_p(mf(&nix::UniformBuffer::update)));
				func_add_param("data", TypePointer);
				func_add_param("size", TypeInt);
			class_add_func("update", TypeVoid, nix_p(mf(&nix::UniformBuffer::update_array)));
				func_add_param("data", TypeDynamicArray);
	
		// drawing
	add_func("NixInit", TypeVoid, nix_p(&nix::Init), FLAG_STATIC);
	/*add_func("NixSetVideoMode", TypeVoid, nix_p(&NixSetVideoMode), FLAG_STATIC);
		func_add_param("api", TypeString);
		func_add_param("xres", TypeInt);
		func_add_param("yres", TypeInt);
		func_add_param("fullscreen",TypeBool);*/
	add_func("NixBindFrameBuffer", TypeVoid, nix_p(&nix::BindFrameBuffer), FLAG_STATIC);
		func_add_param("fb", TypeFrameBuffer);
//	add_func("NixStartFrame", TypeBool, nix_p(&nix::StartFrame), FLAG_STATIC);
//	add_func("NixEndFrame", TypeVoid, nix_p(&nix::EndFrame), FLAG_STATIC);
	//add_func("NixKillWindows", TypeVoid, nix_p(&nix::KillWindows), FLAG_STATIC);
	add_func("NixKill", TypeVoid, nix_p(&nix::Kill), FLAG_STATIC);
	add_func("NixResetToColor", TypeVoid, nix_p(&nix::ResetToColor), FLAG_STATIC);
		func_add_param("c", TypeColor);
	add_func("NixSetWorldMatrix", TypeVoid, nix_p(&nix::SetWorldMatrix), FLAG_STATIC);
		func_add_param("m", TypeMatrix);
	add_func("NixDrawTriangles", TypeVoid, nix_p(&nix::DrawTriangles), FLAG_STATIC);
		func_add_param("vb", TypeVertexBuffer);
	add_func("NixDrawLines", TypeVoid, nix_p(&nix::DrawLines), FLAG_STATIC);
		func_add_param("vb", TypeVertexBuffer);
		func_add_param("contiguous", TypeBool);
	add_func("NixSetAlphaM", TypeVoid, nix_p(&nix::SetAlphaM), FLAG_STATIC);
		func_add_param("mode", TypeInt);
	add_func("NixSetAlphaSD", TypeVoid, nix_p(&nix::SetAlphaSD), FLAG_STATIC);
		func_add_param("source", TypeInt);
		func_add_param("dest", TypeInt);
	add_func("NixSetStencil", TypeVoid, nix_p(&nix::SetStencil), FLAG_STATIC);
		func_add_param("mode", TypeInt);
		func_add_param("param", TypeInt);
	add_func("NixSetProjectionPerspective", TypeVoid, nix_p(&nix::SetProjectionPerspective), FLAG_STATIC);
	add_func("NixSetProjectionPerspectiveExt", TypeVoid, nix_p(&nix::SetProjectionPerspectiveExt), FLAG_STATIC);
		func_add_param("centerx", TypeFloat32);
		func_add_param("centery", TypeFloat32);
		func_add_param("width_1", TypeFloat32);
		func_add_param("height_1", TypeFloat32);
		func_add_param("zmin", TypeFloat32);
		func_add_param("zmax", TypeFloat32);
	add_func("NixSetProjectionOrtho", TypeVoid, nix_p(&nix::SetProjectionOrtho), FLAG_STATIC);
		func_add_param("relative", TypeBool);
	add_func("NixSetProjectionOrthoExt", TypeVoid, nix_p(&nix::SetProjectionOrthoExt), FLAG_STATIC);
		func_add_param("centerx", TypeFloat32);
		func_add_param("centery", TypeFloat32);
		func_add_param("map_width", TypeFloat32);
		func_add_param("map_height", TypeFloat32);
		func_add_param("zmin", TypeFloat32);
		func_add_param("zmax", TypeFloat32);
	add_func("NixSetProjectionMatrix", TypeVoid, nix_p(&nix::SetProjectionMatrix), FLAG_STATIC);
		func_add_param("m", TypeMatrix);
	add_func("NixSetViewMatrix", TypeVoid, nix_p(&nix::SetViewMatrix), FLAG_STATIC);
		func_add_param("view_mat", TypeMatrix);
	add_func("NixSetScissor", TypeVoid, nix_p(&nix::SetScissor), FLAG_STATIC);
		func_add_param("r", TypeRect);
	add_func("NixSetZ", TypeVoid, nix_p(&nix::SetZ), FLAG_STATIC);
		func_add_param("write", TypeBool);
		func_add_param("test", TypeBool);
	add_func("NixSetCull", TypeVoid, nix_p(&nix::SetCull), FLAG_STATIC);
		func_add_param("mode", TypeInt);
	add_func("NixSetWire", TypeVoid, nix_p(&nix::SetWire), FLAG_STATIC);
		func_add_param("enabled", TypeBool);
	add_func("NixSetMaterial", TypeVoid, nix_p(&nix::SetMaterial), FLAG_STATIC);
		func_add_param("ambient", TypeColor);
		func_add_param("diffuse", TypeColor);
		func_add_param("specular", TypeColor);
		func_add_param("shininess", TypeFloat32);
		func_add_param("emission", TypeColor);
	add_func("NixSetTexture", TypeVoid, nix_p(&nix::SetTexture), FLAG_STATIC);
		func_add_param("t", TypeTexture);
	add_func("NixSetShader", TypeVoid, nix_p(&nix::SetShader), FLAG_STATIC);
		func_add_param("s", TypeShader);
	add_func("NixBindUniform", TypeVoid, nix_p(mf(&nix::BindUniform)), FLAG_STATIC);
		func_add_param("buf", TypeUniformBuffer);
		func_add_param("binding", TypeInt);
	add_func("NixScreenShotToImage", TypeVoid, nix_p(&nix::ScreenShotToImage), FLAG_STATIC);
		func_add_param("im", TypeImage);

	add_ext_var("target", TypeRect, nix_p(&nix::target_rect));
	add_ext_var("fullscreen", TypeBool, nix_p(&nix::Fullscreen));

	// alpha operations
	add_const("ALPHA_NONE",             TypeInt, nix_p(ALPHA_NONE));
	add_const("ALPHA_ZERO",             TypeInt, nix_p(ALPHA_ZERO));
	add_const("ALPHA_ONE",              TypeInt, nix_p(ALPHA_ONE));
	add_const("ALPHA_COLOR_KEY",        TypeInt, nix_p(ALPHA_COLOR_KEY_SMOOTH));
	add_const("ALPHA_COLOR_KEY_HARD",   TypeInt, nix_p(ALPHA_COLOR_KEY_HARD));
	add_const("ALPHA_ADD",              TypeInt, nix_p(ALPHA_ADD));
	add_const("ALPHA_MATERIAL",         TypeInt, nix_p(ALPHA_MATERIAL));
	add_const("ALPHA_SOURCE_COLOR",     TypeInt, nix_p(ALPHA_SOURCE_COLOR));
	add_const("ALPHA_SOURCE_INV_COLOR", TypeInt, nix_p(ALPHA_SOURCE_INV_COLOR));
	add_const("ALPHA_SOURCE_ALPHA",     TypeInt, nix_p(ALPHA_SOURCE_ALPHA));
	add_const("ALPHA_SOURCE_INV_ALPHA", TypeInt, nix_p(ALPHA_SOURCE_INV_ALPHA));
	add_const("ALPHA_DEST_COLOR",       TypeInt, nix_p(ALPHA_DEST_COLOR));
	add_const("ALPHA_DEST_INV_COLOR",   TypeInt, nix_p(ALPHA_DEST_INV_COLOR));
	add_const("ALPHA_DEST_ALPHA",       TypeInt, nix_p(ALPHA_DEST_ALPHA));
	add_const("ALPHA_DEST_INV_ALPHA",   TypeInt, nix_p(ALPHA_DEST_INV_ALPHA));
	// stencil operations
	add_const("STENCIL_NONE",               TypeInt, nix_p(STENCIL_NONE));
	add_const("STENCIL_INCREASE",           TypeInt, nix_p(STENCIL_INCREASE));
	add_const("STENCIL_DECREASE",           TypeInt, nix_p(STENCIL_DECREASE));
	add_const("STENCIL_SET",                TypeInt, nix_p(STENCIL_SET));
	add_const("STENCIL_MASK_EQUAL",         TypeInt, nix_p(STENCIL_MASK_EQUAL));
	add_const("STENCIL_MASK_NOT_EQUAL",     TypeInt, nix_p(STENCIL_MASK_NOT_EQUAL));
	add_const("STENCIL_MASK_LESS",          TypeInt, nix_p(STENCIL_MASK_LESS));
	add_const("STENCIL_MASK_LESS_EQUAL",    TypeInt, nix_p(STENCIL_MASK_LESS_EQUAL));
	add_const("STENCIL_MASK_GREATER",       TypeInt, nix_p(STENCIL_MASK_GREATER));
	add_const("STENCIL_MASK_GREATER_EQUAL", TypeInt, nix_p(STENCIL_MASK_GREATER_EQUAL));
	add_const("STENCIL_RESET",              TypeInt, nix_p(STENCIL_RESET));
	// fog
	add_const("FOG_LINEAR", TypeInt, nix_p(FOG_LINEAR));
	add_const("FOG_EXP",    TypeInt, nix_p(FOG_EXP));
	add_const("FOG_EXP2",   TypeInt, nix_p(FOG_EXP2));


	add_ext_var("vb_temp", TypeVertexBufferP, nix_p(&nix::vb_temp));
}

};
