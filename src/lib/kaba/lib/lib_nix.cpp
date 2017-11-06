#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"

#ifdef _X_USE_NIX_
	#include "../../nix/nix.h"
#endif

namespace Kaba{


#ifdef _X_USE_NIX_
	#define nix_p(p)		(void*)p
#else
	namespace nix{
		typedef int VertexBuffer;
		typedef int Texture;
		typedef int Shader;
	};
	#define nix_p(p)		NULL
#endif



extern Class *TypeMatrix;
extern Class *TypeImage;
extern Class *TypeFloatArrayP;
extern Class *TypeVectorArray;
extern Class *TypeVectorArrayP;
Class *TypeVertexBuffer;
Class *TypeVertexBufferP;
Class *TypeTexture;
Class *TypeTextureP;
Class *TypeTexturePList;
Class *TypeDynamicTexture;
Class *TypeCubeMap;
Class *TypeShader;
Class *TypeShaderP;

void SIAddPackageNix()
{
	add_package("nix", false);
	
	TypeVectorArray		= add_type_a("vector[?]",	TypeVector, 1);
	TypeVectorArrayP	= add_type_p("vector[?]*",	TypeVectorArray);
	TypeVertexBuffer	= add_type  ("VertexBuffer", sizeof(nix::VertexBuffer));
	TypeVertexBufferP	= add_type_p("VertexBuffer*",	TypeVertexBuffer);
	TypeTexture			= add_type  ("Texture", sizeof(nix::Texture));
	TypeTextureP		= add_type_p("Texture*",	TypeTexture);
	TypeTexturePList	= add_type_a("Texture*[]",	TypeTextureP, -1);
	TypeDynamicTexture	= add_type  ("DynamicTexture", sizeof(nix::Texture));
	TypeCubeMap			= add_type  ("CubeMap", sizeof(nix::Texture));
	TypeShader			= add_type  ("Shader", sizeof(nix::Shader));
	TypeShaderP			= add_type_p("Shader*",	TypeShader);
	
	add_class(TypeVertexBuffer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::VertexBuffer::__init__)));
			func_add_param("num_textures", TypeInt);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::VertexBuffer::__delete__)));
		class_add_func("clear", TypeVoid, nix_p(mf(&nix::VertexBuffer::clear)));
		class_add_func("addTria",							TypeVoid,	nix_p(mf(&nix::VertexBuffer::addTria)));
			func_add_param("p1",		TypeVector);
			func_add_param("n1",		TypeVector);
			func_add_param("u1",		TypeFloat32);
			func_add_param("v1",		TypeFloat32);
			func_add_param("p2",		TypeVector);
			func_add_param("n2",		TypeVector);
			func_add_param("u2",		TypeFloat32);
			func_add_param("v2",		TypeFloat32);
			func_add_param("p3",		TypeVector);
			func_add_param("n3",		TypeVector);
			func_add_param("u3",		TypeFloat32);
			func_add_param("v3",		TypeFloat32);
		class_add_func("addTrias",							TypeVoid,	nix_p(mf(&nix::VertexBuffer::addTrias)));
			func_add_param("num_trias",		TypeInt);
			func_add_param("p",		TypeVectorArrayP);
			func_add_param("n",		TypeVectorArrayP);
			func_add_param("t",		TypeFloatArrayP);


	add_class(TypeTexture);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::Texture::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, nix_p(mf(&nix::Texture::__delete__)));
		class_add_func("overwrite",	TypeVoid,	nix_p(mf(&nix::Texture::overwrite)));
			func_add_param("image", TypeImage);
		class_add_func("startRender", TypeBool, nix_p(mf(&nix::Texture::start_render)));

	add_class(TypeDynamicTexture);
		TypeDynamicTexture->derive_from(TypeTexture, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::DynamicTexture::__init__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);

	add_class(TypeCubeMap);
		TypeCubeMap->derive_from(TypeTexture, false);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nix_p(mf(&nix::CubeMap::__init__)));
			func_add_param("size", TypeInt);

	add_class(TypeShader);
		class_add_func("unref",										TypeVoid,	nix_p(mf(&nix::Shader::unref)));
		class_add_func("setData",					TypeVoid,	nix_p(mf(&nix::Shader::set_data)));
			func_add_param("name",		TypeString);
			func_add_param("data",		TypePointer);
			func_add_param("size",		TypeInt);
		class_add_func("getData",					TypeVoid,	nix_p(mf(&nix::Shader::get_data)));
			func_add_param("name",		TypeString);
			func_add_param("data",		TypePointer);
			func_add_param("size",		TypeInt);

	add_func("LoadTexture",			TypeTextureP,	nix_p(&nix::LoadTexture));
		func_add_param("filename",		TypeString);
	
		// drawing
	add_func("NixInit",				TypeVoid,					nix_p(&nix::Init));
		func_add_param("api",		TypeString);
		func_add_param("w",			TypeFloat32);
		func_add_param("h",		TypeFloat32);
	/*add_func("NixSetVideoMode",				TypeVoid,					nix_p(&NixSetVideoMode));
		func_add_param("api",		TypeString);
		func_add_param("xres",		TypeInt);
		func_add_param("yres",		TypeInt);
		func_add_param("fullscreen",TypeBool);*/
	add_func("NixStart",									TypeBool,	nix_p(&nix::Start));
	add_func("NixEnd",											TypeVoid,	nix_p(&nix::End));
	//add_func("NixKillWindows",							TypeVoid,	nix_p(&nix::KillWindows));
	add_func("NixResetToColor",							TypeVoid,	nix_p(&nix::ResetToColor));
		func_add_param("c",		TypeColor);
	add_func("NixSetWorldMatrix",								TypeVoid,	nix_p(&nix::SetWorldMatrix));
		func_add_param("m",		TypeMatrix);
	add_func("NixDraw3D",								TypeVoid,	nix_p(&nix::Draw3D));
		func_add_param("vb",		TypeVertexBufferP);
	add_func("NixDraw2D",								TypeVoid,	nix_p(&nix::Draw2D));
		func_add_param("source",		TypeRect);
		func_add_param("dest",		TypeRect);
		func_add_param("z",		TypeFloat32);
	add_func("NixDrawStr",									TypeVoid,	nix_p(&nix::DrawStr));
		func_add_param("x",		TypeFloat32);
		func_add_param("y",		TypeFloat32);
		func_add_param("str",	TypeString);
	add_func("NixDrawLineH",							TypeVoid,	nix_p(&nix::DrawLineH));
		func_add_param("x",		TypeFloat32);
		func_add_param("y1",		TypeFloat32);
		func_add_param("y2",		TypeFloat32);
		func_add_param("z",		TypeFloat32);
	add_func("NixDrawLineV",							TypeVoid,	nix_p(&nix::DrawLineV));
		func_add_param("x1",		TypeFloat32);
		func_add_param("x2",		TypeFloat32);
		func_add_param("y",		TypeFloat32);
		func_add_param("z",		TypeFloat32);
	add_func("NixDrawLine",									TypeVoid,	nix_p(&nix::DrawLine));
		func_add_param("x1",		TypeFloat32);
		func_add_param("y1",		TypeFloat32);
		func_add_param("x2",		TypeFloat32);
		func_add_param("y2",		TypeFloat32);
		func_add_param("z",		TypeFloat32);
	add_func("NixDrawLine3D",									TypeVoid,	nix_p(&nix::DrawLine3D));
		func_add_param("l1",		TypeVector);
		func_add_param("l2",		TypeVector);
	add_func("NixDrawSprite",							TypeVoid,	nix_p(&nix::DrawSprite));
		func_add_param("source",		TypeRect);
		func_add_param("pos",		TypeVector);
		func_add_param("radius",		TypeFloat32);
	//add_func("NixDrawModel2D",						TypeVoid);
	//	func_add_param("???",		TypeFloat); // ???
	add_func("NixSetAlphaM",								TypeVoid,	nix_p(&nix::SetAlphaM));
		func_add_param("mode",		TypeInt);
	add_func("NixSetAlphaSD",						TypeVoid,	nix_p(&nix::SetAlphaSD));
		func_add_param("source",		TypeInt);
		func_add_param("dest",		TypeInt);
	add_func("NixSetStencil",							TypeVoid,	nix_p(&nix::SetStencil));
		func_add_param("mode",		TypeInt);
		func_add_param("param",		TypeInt);
	add_func("NixSetProjectionPerspective",									TypeVoid,	nix_p(&nix::SetProjectionPerspective));
	add_func("NixSetProjectionPerspectiveExt",									TypeVoid,	nix_p(&nix::SetProjectionPerspectiveExt));
		func_add_param("centerx",		TypeFloat32);
		func_add_param("centery",		TypeFloat32);
		func_add_param("width_1",		TypeFloat32);
		func_add_param("height_1",		TypeFloat32);
		func_add_param("zmin",		TypeFloat32);
		func_add_param("zmax",		TypeFloat32);
	add_func("NixSetProjectionOrtho",									TypeVoid,	nix_p(&nix::SetProjectionOrtho));
		func_add_param("relative",		TypeBool);
	add_func("NixSetProjectionOrthoExt",									TypeVoid,	nix_p(&nix::SetProjectionOrthoExt));
		func_add_param("centerx",		TypeFloat32);
		func_add_param("centery",		TypeFloat32);
		func_add_param("map_width",		TypeFloat32);
		func_add_param("map_height",	TypeFloat32);
		func_add_param("zmin",		TypeFloat32);
		func_add_param("zmax",		TypeFloat32);
	add_func("NixSetProjectionMatrix",									TypeVoid,	nix_p(&nix::SetProjectionMatrix));
		func_add_param("m",		TypeMatrix);
	add_func("NixSetViewMatrix",									TypeVoid,	nix_p(&nix::SetViewMatrix));
		func_add_param("view_mat",		TypeMatrix);
	add_func("NixSetViewPosAng",								TypeVoid,	nix_p(&nix::SetViewPosAng));
		func_add_param("pos",		TypeVector);
		func_add_param("ang",		TypeQuaternion);
	add_func("NixScissor",									TypeVoid,	nix_p(&nix::Scissor));
		func_add_param("r",		TypeRect);
	add_func("NixSetZ",											TypeVoid,	nix_p(&nix::SetZ));
		func_add_param("write",		TypeBool);
		func_add_param("test",		TypeBool);
	add_func("NixEnableLighting",											TypeVoid,	nix_p(&nix::EnableLighting));
		func_add_param("enabled",		TypeBool);

	add_func("NixSetLightRadial",										TypeVoid,	nix_p(&nix::SetLightRadial));
		func_add_param("light",			TypeInt);
		func_add_param("pos",			TypeVector);
		func_add_param("radius",			TypeFloat32);
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
	add_func("NixSetLightDirectional",									TypeVoid,	nix_p(&nix::SetLightDirectional));
		func_add_param("light",			TypeInt);
		func_add_param("dir",			TypeVector);
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
	add_func("NixEnableLight",											TypeVoid,	nix_p(&nix::EnableLight));
		func_add_param("light",			TypeInt);
		func_add_param("enabled",		TypeBool);
	add_func("NixSetAmbientLight",										TypeVoid,	nix_p(&nix::SetAmbientLight));
		func_add_param("ambient",		TypeColor);
	add_func("NixSpecularEnable",										TypeVoid,	nix_p(&nix::SpecularEnable));
		func_add_param("enabled",		TypeBool);
	add_func("NixSetCull",											TypeVoid,	nix_p(&nix::SetCull));
		func_add_param("mode",		TypeInt);
	add_func("NixSetWire",											TypeVoid,	nix_p(&nix::SetWire));
		func_add_param("enabled",		TypeBool);
	add_func("NixSetMaterial",							TypeVoid,	nix_p(&nix::SetMaterial));
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
		func_add_param("shininess",		TypeFloat32);
		func_add_param("emission",		TypeColor);
	add_func("NixSetColor",		TypeVoid,	nix_p(&nix::SetColor));
		func_add_param("c",			TypeColor);
	add_func("NixSetTexture",		TypeVoid,	nix_p(&nix::SetTexture));
		func_add_param("t",			TypeTextureP);
	add_func("VecProject",								TypeVoid,	nix_p(&nix::GetVecProject));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("VecUnproject",							TypeVoid,	nix_p(&nix::GetVecUnproject));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("VecProjectRel",						TypeVoid,	nix_p(&nix::GetVecProjectRel));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("VecUnprojectRel",						TypeVoid,	nix_p(&nix::GetVecUnprojectRel));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("NixLoadShader",										TypeShaderP,	nix_p(&nix::LoadShader));
		func_add_param("filename",		TypeString);
	add_func("NixCreateShader",										TypeShaderP,	nix_p(&nix::CreateShader));
		func_add_param("source",		TypeString);
	add_func("ScreenShotExt",								TypeVoid,	nix_p(&nix::ScreenShot));
		func_add_param("filename",	TypeString);
		func_add_param("width",		TypeInt);
		func_add_param("height",		TypeInt);
	add_func("NixScreenShotToImage",								TypeVoid,	nix_p(&nix::ScreenShotToImage));
		func_add_param("im",	TypeImage);

	add_ext_var("TargetWidth",		TypeInt,		nix_p(&nix::target_height));
	add_ext_var("TargetHeight",		TypeInt,		nix_p(&nix::target_height));
	add_ext_var("Target",			TypeRect,		nix_p(&nix::target_rect));
	add_ext_var("Fullscreen",		TypeBool,		nix_p(&nix::Fullscreen));
	add_ext_var("Api",				TypeString,		nix_p(&nix::ApiName));
	//add_ext_var("TextureLifeTime",	TypeInt,		nix_p(&nix::TextureMaxFramesToLive));
	add_ext_var("LineWidth",		TypeFloat32,		nix_p(&nix::line_width));
	add_ext_var("SmoothLines",		TypeBool,		nix_p(&nix::smooth_lines));

	// alpha operations
	add_const("AlphaNone",           TypeInt, nix_p(AlphaNone));
	add_const("AlphaZero",           TypeInt, nix_p(AlphaZero));
	add_const("AlphaOne",            TypeInt, nix_p(AlphaOne));
	add_const("AlphaColorKey",       TypeInt, nix_p(AlphaColorKey));
	add_const("AlphaColorKeyHard",   TypeInt, nix_p(AlphaColorKeyHard));
	add_const("AlphaAdd",            TypeInt, nix_p(AlphaAdd));
	add_const("AlphaMaterial",       TypeInt, nix_p(AlphaMaterial));
	add_const("AlphaSourceColor",    TypeInt, nix_p(AlphaSourceColor));
	add_const("AlphaSourceInvColor", TypeInt, nix_p(AlphaSourceInvColor));
	add_const("AlphaSourceAlpha",    TypeInt, nix_p(AlphaSourceAlpha));
	add_const("AlphaSourceInvAlpha", TypeInt, nix_p(AlphaSourceInvAlpha));
	add_const("AlphaDestColor",      TypeInt, nix_p(AlphaDestColor));
	add_const("AlphaDestInvColor",   TypeInt, nix_p(AlphaDestInvColor));
	add_const("AlphaDestAlpha",      TypeInt, nix_p(AlphaDestAlpha));
	add_const("AlphaDestInvAlpha",   TypeInt, nix_p(AlphaDestInvAlpha));
	// stencil operations
	add_const("StencilNone",             TypeInt, nix_p(StencilNone));
	add_const("StencilIncrease",         TypeInt, nix_p(StencilIncrease));
	add_const("StencilDecrease",         TypeInt, nix_p(StencilDecrease));
	add_const("StencilSet",              TypeInt, nix_p(StencilSet));
	add_const("StencilMaskEqual",        TypeInt, nix_p(StencilMaskEqual));
	add_const("StencilMaskNotEqual",     TypeInt, nix_p(StencilMaskNotEqual));
	add_const("StencilMaskLess",         TypeInt, nix_p(StencilMaskLess));
	add_const("StencilMaskLessEqual",    TypeInt, nix_p(StencilMaskLessEqual));
	add_const("StencilMaskGreater",      TypeInt, nix_p(StencilMaskGreater));
	add_const("StencilMaskGreaterEqual", TypeInt, nix_p(StencilMaskGreaterEqual));
	add_const("StencilReset",            TypeInt, nix_p(StencilReset));
	// fog
	add_const("FogLinear", TypeInt, nix_p(FogLinear));
	add_const("FogExp",    TypeInt, nix_p(FogExp));
	add_const("FogExp2",   TypeInt, nix_p(FogExp2));

	add_ext_var("VBTemp",     TypeVertexBufferP, nix_p(nix::vb_temp));
}

};
