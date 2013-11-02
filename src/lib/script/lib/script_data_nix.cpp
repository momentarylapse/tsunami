#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

#ifdef _X_USE_NIX_
	#include "../../nix/nix.h"
#endif

namespace Script{


#ifdef _X_USE_NIX_
	#define nix_p(p)		(void*)p
#else
	typedef int NixVertexBuffer;
	typedef int NixTexture;
	typedef int NixShader;
	#define nix_p(p)		NULL
#endif



extern Type *TypeMatrix;
extern Type *TypeImage;
extern Type *TypeFloatArrayP;
extern Type *TypeVectorArray;
extern Type *TypeVectorArrayP;
extern Type *TypeHuiWindowP; // -> script_data_hui.cpp
Type *TypeVertexBuffer;
Type *TypeVertexBufferP;
Type *TypeTexture;
Type *TypeTextureP;
Type *TypeTexturePArray;
Type *TypeDynamicTexture;
Type *TypeCubeMap;
Type *TypeShader;
Type *TypeShaderP;

void SIAddPackageNix()
{
	msg_db_f("SIAddPackageNix", 3);
	
	add_package("nix", false);
	
	TypeVectorArray		= add_type_a("vector[?]",	TypeVector, 1);
	TypeVectorArrayP	= add_type_p("vector[?]*",	TypeVectorArray);
	TypeVertexBuffer	= add_type  ("VertexBuffer", sizeof(NixVertexBuffer));
	TypeVertexBufferP	= add_type_p("VertexBuffer*",	TypeVertexBuffer);
	TypeTexture			= add_type  ("Texture", sizeof(NixTexture));
	TypeTextureP		= add_type_p("Texture*",	TypeTexture);
	TypeTexturePArray	= add_type_a("Texture*[?]",	TypeTextureP, 1);
	TypeDynamicTexture	= add_type  ("DynamicTexture", sizeof(NixTexture));
	TypeCubeMap			= add_type  ("CubeMap", sizeof(NixTexture));
	TypeShader			= add_type  ("Shader", sizeof(NixShader));
	TypeShaderP			= add_type_p("Shader*",	TypeShader);
	
	add_class(TypeVertexBuffer);
		class_add_func("__init__", TypeVoid, nix_p(mf(&NixVertexBuffer::__init__)));
			func_add_param("num_textures", TypeInt);
		class_add_func("__delete__", TypeVoid, nix_p(mf(&NixVertexBuffer::__delete__)));
		class_add_func("clear", TypeVoid, nix_p(mf(&NixVertexBuffer::clear)));
		class_add_func("addTria",							TypeVoid,	nix_p(mf(&NixVertexBuffer::addTria)));
			func_add_param("p1",		TypeVector);
			func_add_param("n1",		TypeVector);
			func_add_param("u1",		TypeFloat);
			func_add_param("v1",		TypeFloat);
			func_add_param("p2",		TypeVector);
			func_add_param("n2",		TypeVector);
			func_add_param("u2",		TypeFloat);
			func_add_param("v2",		TypeFloat);
			func_add_param("p3",		TypeVector);
			func_add_param("n3",		TypeVector);
			func_add_param("u3",		TypeFloat);
			func_add_param("v3",		TypeFloat);
		class_add_func("addTrias",							TypeVoid,	nix_p(mf(&NixVertexBuffer::addTrias)));
			func_add_param("num_trias",		TypeInt);
			func_add_param("p",		TypeVectorArrayP);
			func_add_param("n",		TypeVectorArrayP);
			func_add_param("t",		TypeFloatArrayP);
		class_add_func("draw", TypeVoid, nix_p(mf(&NixVertexBuffer::draw)));


	add_class(TypeTexture);
		class_add_func("__init__", TypeVoid, nix_p(mf(&NixTexture::__init__)));
		class_add_func("__delete__", TypeVoid, nix_p(mf(&NixTexture::__delete__)));
		class_add_func("overwrite",	TypeVoid,	nix_p(mf(&NixTexture::overwrite)));
			func_add_param("image", TypeImage);
		class_add_func("startRender", TypeBool, nix_p(mf(&NixTexture::start_render)));

	add_class(TypeDynamicTexture);
		TypeDynamicTexture->DeriveFrom(TypeTexture, false);
		class_add_func("__init__", TypeVoid, nix_p(mf(&NixDynamicTexture::__init__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);

	add_class(TypeCubeMap);
		TypeCubeMap->DeriveFrom(TypeTexture, false);
		class_add_func("__init__", TypeVoid, nix_p(mf(&NixCubeMap::__init__)));
			func_add_param("size", TypeInt);

	add_class(TypeShader);
		class_add_func("unref",										TypeVoid,	nix_p(mf(&NixShader::unref)));
		class_add_func("setData",					TypeVoid,	nix_p(mf(&NixShader::set_data)));
			func_add_param("name",		TypeString);
			func_add_param("data",		TypePointer);
			func_add_param("size",		TypeInt);
		class_add_func("getData",					TypeVoid,	nix_p(mf(&NixShader::get_data)));
			func_add_param("name",		TypeString);
			func_add_param("data",		TypePointer);
			func_add_param("size",		TypeInt);

	add_func("LoadTexture",			TypeTextureP,	nix_p(&NixLoadTexture));
		func_add_param("filename",		TypeString);
	
		// user input
	add_func("NixUpdateInput",			TypeVoid,	nix_p(&NixUpdateInput));
	add_func("GetKey",								TypeBool,	nix_p(&NixGetKey));
		func_add_param("id",		TypeInt);
	add_func("GetKeyDown",								TypeBool,	nix_p(&NixGetKeyDown));
		func_add_param("id",		TypeInt);
	add_func("GetKeyDownRep",							TypeInt,	nix_p(&NixGetKeyDownRep));
	add_func("GetKeyUp",								TypeBool,	nix_p(&NixGetKeyUp));
		func_add_param("id",		TypeInt);
	add_func("GetKeyChar",							TypeString,	nix_p(&NixGetKeyChar));
		func_add_param("id",		TypeInt);
	add_func("GetButton",										TypeBool,	nix_p(&NixGetButton));
		func_add_param("button",	TypeInt);
	add_func("GetButtonDown",										TypeBool,	nix_p(&NixGetButtonDown));
		func_add_param("button",	TypeInt);
	add_func("GetButtonUp",										TypeBool,	nix_p(&NixGetButtonUp));
		func_add_param("button",	TypeInt);
		// drawing
	add_func("NixInit",				TypeVoid,					nix_p(&NixInit));
		func_add_param("api",		TypeString);
		func_add_param("w",			TypeHuiWindowP);
		func_add_param("id",		TypeString);
	add_func("NixSetVideoMode",				TypeVoid,					nix_p(&NixSetVideoMode));
		func_add_param("api",		TypeString);
		func_add_param("xres",		TypeInt);
		func_add_param("yres",		TypeInt);
		func_add_param("fullscreen",TypeBool);
	add_func("NixStart",									TypeBool,	nix_p(&NixStart));
	add_func("NixEnd",											TypeVoid,	nix_p(&NixEnd));
	add_func("NixKillWindows",							TypeVoid,	nix_p(&NixKillWindows));
	add_func("NixResetToColor",							TypeVoid,	nix_p(&NixResetToColor));
		func_add_param("c",		TypeColor);
	add_func("NixSetWorldMatrix",								TypeVoid,	nix_p(&NixSetWorldMatrix));
		func_add_param("m",		TypeMatrix);
	add_func("NixDraw2D",								TypeVoid,	nix_p(&NixDraw2D));
		func_add_param("source",		TypeRect);
		func_add_param("dest",		TypeRect);
		func_add_param("z",		TypeFloat);
	add_func("NixDrawStr",									TypeVoid,	nix_p(&NixDrawStr));
		func_add_param("x",		TypeFloat);
		func_add_param("y",		TypeFloat);
		func_add_param("str",	TypeString);
	add_func("NixDrawLineH",							TypeVoid,	nix_p(&NixDrawLineH));
		func_add_param("x",		TypeFloat);
		func_add_param("y1",		TypeFloat);
		func_add_param("y2",		TypeFloat);
		func_add_param("z",		TypeFloat);
	add_func("NixDrawLineV",							TypeVoid,	nix_p(&NixDrawLineV));
		func_add_param("x1",		TypeFloat);
		func_add_param("x2",		TypeFloat);
		func_add_param("y",		TypeFloat);
		func_add_param("z",		TypeFloat);
	add_func("NixDrawLine",									TypeVoid,	nix_p(&NixDrawLine));
		func_add_param("x1",		TypeFloat);
		func_add_param("y1",		TypeFloat);
		func_add_param("x2",		TypeFloat);
		func_add_param("y2",		TypeFloat);
		func_add_param("z",		TypeFloat);
	add_func("NixDrawLine3D",									TypeVoid,	nix_p(&NixDrawLine3D));
		func_add_param("l1",		TypeVector);
		func_add_param("l2",		TypeVector);
	add_func("NixDrawSprite",							TypeVoid,	nix_p(&NixDrawSprite));
		func_add_param("source",		TypeRect);
		func_add_param("pos",		TypeVector);
		func_add_param("radius",		TypeFloat);
	//add_func("NixDrawModel2D",						TypeVoid);
	//	func_add_param("???",		TypeFloat); // ???
	add_func("NixSetAlphaM",								TypeVoid,	nix_p(&NixSetAlphaM));
		func_add_param("mode",		TypeInt);
	add_func("NixSetAlphaSD",						TypeVoid,	nix_p(&NixSetAlphaSD));
		func_add_param("source",		TypeInt);
		func_add_param("dest",		TypeInt);
	add_func("NixSetStencil",							TypeVoid,	nix_p(&NixSetStencil));
		func_add_param("mode",		TypeInt);
		func_add_param("param",		TypeInt);
	add_func("NixSetProjectionPerspective",									TypeVoid,	nix_p(&NixSetProjectionPerspective));
	add_func("NixSetProjectionPerspectiveExt",									TypeVoid,	nix_p(&NixSetProjectionPerspectiveExt));
		func_add_param("centerx",		TypeFloat);
		func_add_param("centery",		TypeFloat);
		func_add_param("width_1",		TypeFloat);
		func_add_param("height_1",		TypeFloat);
		func_add_param("zmin",		TypeFloat);
		func_add_param("zmax",		TypeFloat);
	add_func("NixSetProjectionOrtho",									TypeVoid,	nix_p(&NixSetProjectionOrtho));
		func_add_param("relative",		TypeBool);
	add_func("NixSetProjectionOrthoExt",									TypeVoid,	nix_p(&NixSetProjectionOrthoExt));
		func_add_param("centerx",		TypeFloat);
		func_add_param("centery",		TypeFloat);
		func_add_param("map_width",		TypeFloat);
		func_add_param("map_height",	TypeFloat);
		func_add_param("zmin",		TypeFloat);
		func_add_param("zmax",		TypeFloat);
	add_func("NixSetProjectionMatrix",									TypeVoid,	nix_p(&NixSetProjectionMatrix));
		func_add_param("m",		TypeMatrix);
	add_func("NixSetViewM",									TypeVoid,	nix_p(&NixSetViewM));
		func_add_param("view_mat",		TypeMatrix);
	add_func("NixSetViewV",								TypeVoid,	nix_p(&NixSetViewV));
		func_add_param("pos",		TypeVector);
		func_add_param("ang",		TypeVector);
	add_func("NixScissor",									TypeVoid,	nix_p(&NixScissor));
		func_add_param("r",		TypeRect);
	add_func("NixSetZ",											TypeVoid,	nix_p(&NixSetZ));
		func_add_param("write",		TypeBool);
		func_add_param("test",		TypeBool);
	add_func("NixEnableLighting",											TypeVoid,	nix_p(&NixEnableLighting));
		func_add_param("enabled",		TypeBool);

	add_func("NixSetLightRadial",										TypeVoid,	nix_p(&NixSetLightRadial));
		func_add_param("light",			TypeInt);
		func_add_param("pos",			TypeVector);
		func_add_param("radius",			TypeFloat);
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
	add_func("NixSetLightDirectional",									TypeVoid,	nix_p(&NixSetLightDirectional));
		func_add_param("light",			TypeInt);
		func_add_param("dir",			TypeVector);
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
	add_func("NixEnableLight",											TypeVoid,	nix_p(&NixEnableLight));
		func_add_param("light",			TypeInt);
		func_add_param("enabled",		TypeBool);
	add_func("NixSetAmbientLight",										TypeVoid,	nix_p(&NixSetAmbientLight));
		func_add_param("ambient",		TypeColor);
	add_func("NixSpecularEnable",										TypeVoid,	nix_p(&NixSpecularEnable));
		func_add_param("enabled",		TypeBool);
	add_func("NixSetCull",											TypeVoid,	nix_p(&NixSetCull));
		func_add_param("mode",		TypeInt);
	add_func("NixSetWire",											TypeVoid,	nix_p(&NixSetWire));
		func_add_param("enabled",		TypeBool);
	add_func("NixSetMaterial",							TypeVoid,	nix_p(&NixSetMaterial));
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
		func_add_param("shininess",		TypeFloat);
		func_add_param("emission",		TypeColor);
	add_func("NixSetColor",		TypeVoid,	nix_p(&NixSetColor));
		func_add_param("c",			TypeColor);
	add_func("NixSetTexture",		TypeVoid,	nix_p(&NixSetTexture));
		func_add_param("t",			TypeTextureP);
	add_func("VecProject",								TypeVoid,	nix_p(&NixGetVecProject));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("VecUnproject",							TypeVoid,	nix_p(&NixGetVecUnproject));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("VecProjectRel",						TypeVoid,	nix_p(&NixGetVecProjectRel));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("VecUnprojectRel",						TypeVoid,	nix_p(&NixGetVecUnprojectRel));
		func_add_param("v_out",		TypeVector);
		func_add_param("v_in",		TypeVector);
	add_func("NixLoadShader",										TypeShaderP,	nix_p(&NixLoadShader));
		func_add_param("filename",		TypeString);
	add_func("NixCreateShader",										TypeShaderP,	nix_p(&NixCreateShader));
		func_add_param("source",		TypeString);
	add_func("ScreenShotExt",								TypeVoid,	nix_p(&NixScreenShot));
		func_add_param("filename",	TypeString);
		func_add_param("width",		TypeInt);
		func_add_param("height",		TypeInt);
	add_func("NixScreenShotToImage",								TypeVoid,	nix_p(&NixScreenShotToImage));
		func_add_param("im",	TypeImage);

	add_ext_var("TargetWidth",		TypeInt,		nix_p(&NixTargetWidth));
	add_ext_var("TargetHeight",		TypeInt,		nix_p(&NixTargetHeight));
	add_ext_var("Target",			TypeRect,		nix_p(&NixTargetRect));
	add_ext_var("ScreenWidth",		TypeInt,		nix_p(&NixScreenWidth));
	add_ext_var("ScreenHeight",		TypeInt,		nix_p(&NixScreenHeight));
	add_ext_var("ScreenDepth",		TypeInt,		nix_p(&NixScreenDepth));
	add_ext_var("Fullscreen",		TypeBool,		nix_p(&NixFullscreen));
	add_ext_var("Api",				TypeString,		nix_p(&NixApiName));
	add_ext_var("Mouse",			TypeVector,		nix_p(&NixMouse));
	add_ext_var("MouseRel",			TypeVector,		nix_p(&NixMouseRel));
	add_ext_var("MouseD",			TypeVector,		nix_p(&NixMouseD));
	add_ext_var("MouseDRel",		TypeVector,		nix_p(&NixMouseDRel));
	add_ext_var("TextureLifeTime",	TypeInt,		nix_p(&NixTextureMaxFramesToLive));
	add_ext_var("LineWidth",		TypeFloat,		nix_p(&NixLineWidth));
	add_ext_var("SmoothLines",		TypeBool,		nix_p(&NixSmoothLines));

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

	add_ext_var("VBTemp",     TypeVertexBufferP, nix_p(VBTemp));
}

};
