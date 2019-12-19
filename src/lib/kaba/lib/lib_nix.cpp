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

nix::Texture* __LoadTexture(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return nix::LoadTexture(filename));
	return nullptr;
}

#pragma GCC pop_options



#else
	namespace nix{
		typedef int VertexBuffer;
		typedef int Texture;
		typedef int Shader;
	};
	#define nix_p(p)		nullptr
#endif



extern const Class *TypeMatrix;
extern const Class *TypeImage;
extern const Class *TypeFloatList;
extern const Class *TypeFloatArrayP;
extern const Class *TypeVectorArray;
extern const Class *TypeVectorArrayP;
const Class *TypeVertexBuffer;
const Class *TypeVertexBufferP;
const Class *TypeTexture;
const Class *TypeTextureP;
const Class *TypeTexturePList;
const Class *TypeDynamicTexture;
const Class *TypeImageTexture;
const Class *TypeDepthTexture;
const Class *TypeCubeMap;
const Class *TypeShader;
const Class *TypeShaderP;

void SIAddPackageNix()
{
	add_package("nix", false);
	
	TypeVectorArray		= add_type_a("vector[?]", TypeVector, 1);
	TypeVectorArrayP	= add_type_p("vector[?]*", TypeVectorArray);
	TypeVertexBuffer	= add_type  ("VertexBuffer", sizeof(nix::VertexBuffer));
	TypeVertexBufferP	= add_type_p("VertexBuffer*", TypeVertexBuffer);
	TypeTexture			= add_type  ("Texture", sizeof(nix::Texture));
	TypeTextureP		= add_type_p("Texture*", TypeTexture);
	TypeTexturePList	= add_type_a("Texture*[]", TypeTextureP, -1);
	TypeDynamicTexture	= add_type  ("DynamicTexture", sizeof(nix::Texture));
	TypeImageTexture	= add_type  ("ImageTexture", sizeof(nix::Texture));
	TypeDepthTexture	= add_type  ("DepthTexture", sizeof(nix::Texture));
	TypeCubeMap			= add_type  ("CubeMap", sizeof(nix::Texture));
	TypeShader			= add_type  ("Shader", sizeof(nix::Shader));
	TypeShaderP			= add_type_p("Shader*", TypeShader);
	
	add_class(TypeVertexBuffer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::VertexBuffer::__init__)));
			func_add_param("num_textures", TypeInt);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::VertexBuffer::__delete__)));
		class_add_func("clear", TypeVoid, nix_p(mf(&nix::VertexBuffer::clear)));
		class_add_func("add_tria", TypeVoid, nix_p(mf(&nix::VertexBuffer::addTria)));
			func_add_param("p1", TypeVector);
			func_add_param("n1", TypeVector);
			func_add_param("u1", TypeFloat32);
			func_add_param("v1", TypeFloat32);
			func_add_param("p2", TypeVector);
			func_add_param("n2", TypeVector);
			func_add_param("u2", TypeFloat32);
			func_add_param("v2", TypeFloat32);
			func_add_param("p3", TypeVector);
			func_add_param("n3", TypeVector);
			func_add_param("u3", TypeFloat32);
			func_add_param("v3", TypeFloat32);
		class_add_func("add_trias", TypeVoid, nix_p(mf(&nix::VertexBuffer::addTrias)));
			func_add_param("num_trias", TypeInt);
			func_add_param("p", TypeVectorArrayP);
			func_add_param("n", TypeVectorArrayP);
			func_add_param("t", TypeFloatArrayP);


	add_class(TypeTexture);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::Texture::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::Texture::__delete__)));
		class_add_func("overwrite", TypeVoid, nix_p(mf(&nix::Texture::overwrite)));
			func_add_param("image", TypeImage);
			class_add_func("read", TypeVoid, nix_p(mf(&nix::Texture::read)));
				func_add_param("image", TypeImage);
			class_add_func("read_float", TypeVoid, nix_p(mf(&nix::Texture::read_float)));
				func_add_param("data", TypeFloatList);

	add_class(TypeDynamicTexture);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::DynamicTexture::__init__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);

	add_class(TypeImageTexture);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::ImageTexture::__init__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("format", TypeString);

	add_class(TypeDepthTexture);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::DepthTexture::__init__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);

	add_class(TypeCubeMap);
		class_derive_from(TypeTexture, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::CubeMap::__init__)));
			func_add_param("size", TypeInt);

	add_class(TypeShader);
		class_add_func("unref", TypeVoid, nix_p(mf(&nix::Shader::unref)));
		class_add_func("location", TypeInt, nix_p(mf(&nix::Shader::get_location)));
			func_add_param("name", TypeString);
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
		class_add_const("DEFAULT_3D", TypeShaderP, nix_p(&nix::default_shader_3d));
		class_add_const("DEFAULT_2D", TypeShaderP, nix_p(&nix::default_shader_2d));

	add_func("LoadTexture", TypeTextureP, nix_p(&__LoadTexture), FLAG_STATIC);
		func_add_param("filename", TypeString);
	
		// drawing
	add_func("NixInit", TypeVoid, nix_p(&nix::Init), FLAG_STATIC);
		func_add_param("api", TypeString);
		func_add_param("w", TypeInt);
		func_add_param("h", TypeInt);
	/*add_func("NixSetVideoMode", TypeVoid, nix_p(&NixSetVideoMode), FLAG_STATIC);
		func_add_param("api", TypeString);
		func_add_param("xres", TypeInt);
		func_add_param("yres", TypeInt);
		func_add_param("fullscreen",TypeBool);*/
	add_func("NixStart", TypeBool, nix_p(&nix::Start), FLAG_STATIC);
	add_func("NixEnd", TypeVoid, nix_p(&nix::End), FLAG_STATIC);
	//add_func("NixKillWindows", TypeVoid, nix_p(&nix::KillWindows), FLAG_STATIC);
	add_func("NixResetToColor", TypeVoid, nix_p(&nix::ResetToColor), FLAG_STATIC);
		func_add_param("c", TypeColor);
	add_func("NixSetWorldMatrix", TypeVoid, nix_p(&nix::SetWorldMatrix), FLAG_STATIC);
		func_add_param("m", TypeMatrix);
	add_func("NixDraw3D", TypeVoid, nix_p(&nix::Draw3D), FLAG_STATIC);
		func_add_param("vb", TypeVertexBufferP);
	add_func("NixDraw2D", TypeVoid, nix_p(&nix::Draw2D), FLAG_STATIC);
		func_add_param("source", TypeRect);
		func_add_param("dest", TypeRect);
		func_add_param("z", TypeFloat32);
	add_func("NixDrawStr", TypeVoid, nix_p(&nix::DrawStr), FLAG_STATIC);
		func_add_param("x", TypeFloat32);
		func_add_param("y", TypeFloat32);
		func_add_param("str", TypeString);
	add_func("NixDrawLineH", TypeVoid, nix_p(&nix::DrawLineH), FLAG_STATIC);
		func_add_param("x", TypeFloat32);
		func_add_param("y1", TypeFloat32);
		func_add_param("y2", TypeFloat32);
		func_add_param("z", TypeFloat32);
	add_func("NixDrawLineV", TypeVoid, nix_p(&nix::DrawLineV), FLAG_STATIC);
		func_add_param("x1", TypeFloat32);
		func_add_param("x2", TypeFloat32);
		func_add_param("y", TypeFloat32);
		func_add_param("z", TypeFloat32);
	add_func("NixDrawLine", TypeVoid, nix_p(&nix::DrawLine), FLAG_STATIC);
		func_add_param("x1", TypeFloat32);
		func_add_param("y1", TypeFloat32);
		func_add_param("x2", TypeFloat32);
		func_add_param("y2", TypeFloat32);
		func_add_param("z", TypeFloat32);
	add_func("NixDrawLine3D", TypeVoid, nix_p(&nix::DrawLine3D), FLAG_STATIC);
		func_add_param("l1", TypeVector);
		func_add_param("l2", TypeVector);
	add_func("NixDrawSprite", TypeVoid, nix_p(&nix::DrawSprite), FLAG_STATIC);
		func_add_param("source", TypeRect);
		func_add_param("pos", TypeVector);
		func_add_param("radius", TypeFloat32);
	//add_func("NixDrawModel2D", TypeVoid, FLAG_STATIC);
	//	func_add_param("???", TypeFloat); // ???
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
	add_func("NixSetViewPosAng", TypeVoid, nix_p(&nix::SetViewPosAng), FLAG_STATIC);
		func_add_param("pos", TypeVector);
		func_add_param("ang", TypeQuaternion);
	add_func("NixScissor", TypeVoid, nix_p(&nix::Scissor), FLAG_STATIC);
		func_add_param("r", TypeRect);
	add_func("NixSetZ", TypeVoid, nix_p(&nix::SetZ), FLAG_STATIC);
		func_add_param("write", TypeBool);
		func_add_param("test", TypeBool);
	add_func("NixEnableLighting", TypeVoid, nix_p(&nix::EnableLighting), FLAG_STATIC);
		func_add_param("enabled", TypeBool);

	add_func("NixSetLightRadial", TypeVoid, nix_p(&nix::SetLightRadial), FLAG_STATIC);
		func_add_param("light", TypeInt);
		func_add_param("pos", TypeVector);
		func_add_param("radius", TypeFloat32);
		func_add_param("diffuse", TypeColor);
		func_add_param("ambient", TypeFloat32);
		func_add_param("specular", TypeFloat32);
	add_func("NixSetLightDirectional", TypeVoid, nix_p(&nix::SetLightDirectional), FLAG_STATIC);
		func_add_param("light", TypeInt);
		func_add_param("dir", TypeVector);
		func_add_param("diffuse", TypeColor);
		func_add_param("ambient", TypeFloat32);
		func_add_param("specular", TypeFloat32);
	add_func("NixEnableLight", TypeVoid, nix_p(&nix::EnableLight), FLAG_STATIC);
		func_add_param("light", TypeInt);
		func_add_param("enabled", TypeBool);
	add_func("NixSetAmbientLight", TypeVoid, nix_p(&nix::SetAmbientLight), FLAG_STATIC);
		func_add_param("ambient", TypeColor);
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
	add_func("NixSetColor", TypeVoid, nix_p(&nix::SetColor), FLAG_STATIC);
		func_add_param("c", TypeColor);
	add_func("NixSetTexture", TypeVoid, nix_p(&nix::SetTexture), FLAG_STATIC);
		func_add_param("t", TypeTexture);
	add_func("NixSetShader", TypeVoid, nix_p(&nix::SetShader), FLAG_STATIC);
		func_add_param("s", TypeShader);
	add_func("VecProject", TypeVoid, nix_p(&nix::GetVecProject), FLAG_STATIC);
		func_add_param("v_out", TypeVector);
		func_add_param("v_in", TypeVector);
	add_func("VecUnproject", TypeVoid, nix_p(&nix::GetVecUnproject), FLAG_STATIC);
		func_add_param("v_out", TypeVector);
		func_add_param("v_in", TypeVector);
	add_func("VecProjectRel", TypeVoid, nix_p(&nix::GetVecProjectRel), FLAG_STATIC);
		func_add_param("v_out", TypeVector);
		func_add_param("v_in", TypeVector);
	add_func("VecUnprojectRel", TypeVoid, nix_p(&nix::GetVecUnprojectRel), FLAG_STATIC);
		func_add_param("v_out", TypeVector);
		func_add_param("v_in", TypeVector);
	add_func("NixLoadShader", TypeShaderP, nix_p(&nix::LoadShader), FLAG_STATIC);
		func_add_param("filename", TypeString);
	add_func("NixCreateShader", TypeShaderP, nix_p(&nix::CreateShader), FLAG_STATIC);
		func_add_param("source", TypeString);
	add_func("ScreenShotExt", TypeVoid, nix_p(&nix::ScreenShot), FLAG_STATIC);
		func_add_param("filename", TypeString);
		func_add_param("width", TypeInt);
		func_add_param("height", TypeInt);
	add_func("NixScreenShotToImage", TypeVoid, nix_p(&nix::ScreenShotToImage), FLAG_STATIC);
		func_add_param("im", TypeImage);

	add_ext_var("target_width", TypeInt, nix_p(&nix::target_height));
	add_ext_var("target_height", TypeInt, nix_p(&nix::target_height));
	add_ext_var("target", TypeRect, nix_p(&nix::target_rect));
	add_ext_var("fullscreen", TypeBool, nix_p(&nix::Fullscreen));
	add_ext_var("Api", TypeString, nix_p(&nix::ApiName));
	//add_ext_var("TextureLifeTime", TypeInt, nix_p(&nix::TextureMaxFramesToLive));
	//add_ext_var("LineWidth", TypeFloat32, nix_p(&nix::line_width));
	//add_ext_var("SmoothLines", TypeBool, nix_p(&nix::smooth_lines));

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
