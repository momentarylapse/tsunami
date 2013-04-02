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
	#define nix_p(p)		NULL
#endif



extern Type *TypeMatrix;
extern Type *TypeImage;
extern Type *TypeFloatArrayP;
extern Type *TypeVectorArray;
extern Type *TypeVectorArrayP;
extern Type *TypeHuiWindowP; // -> script_data_hui.cpp

void SIAddPackageNix()
{
	msg_db_f("SIAddPackageNix", 3);
	
	set_cur_package("nix");
	
	TypeVectorArray		= add_type_a("vector[?]",	TypeVector, 1);
	TypeVectorArrayP	= add_type_p("vector[?]*",	TypeVectorArray);
	
	add_func("LoadTexture",			TypeInt,	nix_p(&NixLoadTexture));
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
		func_add_param("xres",		TypeInt);
		func_add_param("yres",		TypeInt);
		func_add_param("depth",		TypeInt);
		func_add_param("fullscreen",TypeBool);
		func_add_param("w",			TypeHuiWindowP);
	add_func("NixStart",									TypeVoid,	nix_p(&NixStart));
		func_add_param("texture",		TypeInt);
	add_func("NixEnd",											TypeVoid,	nix_p(&NixEnd));
	add_func("NixKillWindows",							TypeVoid,	nix_p(&NixKillWindows));
	add_func("NixResetToColor",							TypeVoid,	nix_p(&NixResetToColor));
		func_add_param("c",		TypeColor);
	add_func("NixSetTexture",								TypeVoid,	nix_p(&NixSetTexture));
		func_add_param("texture",		TypeInt);
	add_func("NixSetWorldMatrix",								TypeVoid,	nix_p(&NixSetWorldMatrix));
		func_add_param("m",		TypeMatrix);
	add_func("NixDraw2D",								TypeVoid,	nix_p(&NixDraw2D));
		func_add_param("source",		TypeRect);
		func_add_param("dest",		TypeRect);
		func_add_param("z",		TypeFloat);
	add_func("NixDraw3D",								TypeVoid,	nix_p(&NixDraw3D));
		func_add_param("vb",		TypeInt);
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
	add_func("NixSetProjection",									TypeVoid,	nix_p(&NixSetProjection));
		func_add_param("perspective",		TypeBool);
		func_add_param("relative",		TypeBool);
	add_func("NixSetViewM",									TypeVoid,	nix_p(&NixSetViewM));
		func_add_param("view_mat",		TypeMatrix);
	add_func("NixSetViewV",								TypeVoid,	nix_p(&NixSetViewV));
		func_add_param("pos",		TypeVector);
		func_add_param("ang",		TypeVector);
	add_func("NixSetZ",											TypeVoid,	nix_p(&NixSetZ));
		func_add_param("write",		TypeBool);
		func_add_param("test",		TypeBool);
	add_func("NixSetPerspectiveMode",											TypeVoid,	nix_p(&NixSetPerspectiveMode));
		func_add_param("mode",		TypeInt);
		func_add_param("param1",	TypeFloat);
		func_add_param("param2",	TypeFloat);
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
	add_func("NixSetVideoMode",		TypeVoid,	nix_p(&NixSetVideoMode));
		func_add_param("api",		TypeInt);
		func_add_param("width",		TypeInt);
		func_add_param("height",	TypeInt);
		func_add_param("depth",		TypeInt);
		func_add_param("fullscreen",TypeBool);
	add_func("NixCreateDynamicTexture",	TypeInt,	nix_p(&NixCreateDynamicTexture));
		func_add_param("width",		TypeInt);
		func_add_param("height",	TypeInt);
	add_func("NixCreateEmptyTexture",	TypeInt,	nix_p(&NixCreateEmptyTexture));
	add_func("NixOverwriteTexture",	TypeVoid,	nix_p(&NixOverwriteTexture));
		func_add_param("tex",		TypeInt);
		func_add_param("image",		TypeImage);
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
	add_func("NixCreateVB",									TypeInt,	nix_p(&NixCreateVB));
		func_add_param("max_trias",		TypeInt);
		func_add_param("num_textures",	TypeInt);
	add_func("NixVBClear",									TypeVoid,	nix_p(&NixVBClear));
		func_add_param("vb",		TypeInt);
	add_func("NixVBAddTria",							TypeVoid,	nix_p(&NixVBAddTria));
		func_add_param("vb",		TypeInt);
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
	add_func("NixVBAddTrias",							TypeVoid,	nix_p(&NixVBAddTrias));
		func_add_param("vb",		TypeInt);
		func_add_param("num_trias",		TypeInt);
		func_add_param("p",		TypeVectorArrayP);
		func_add_param("n",		TypeVectorArrayP);
		func_add_param("t",		TypeFloatArrayP);
	add_func("NixLoadShader",										TypeInt,	nix_p(&NixLoadShader));
		func_add_param("filename",		TypeString);
	add_func("NixCreateShader",										TypeInt,	nix_p(&NixCreateShader));
		func_add_param("source",		TypeString);
	add_func("NixUnrefShader",										TypeVoid,	nix_p(&NixUnrefShader));
		func_add_param("index",		TypeInt);
	add_func("NixSetShaderData",					TypeVoid,	nix_p(&NixSetShaderData));
		func_add_param("index",		TypeInt);
		func_add_param("name",		TypeString);
		func_add_param("data",		TypePointer);
		func_add_param("size",		TypeInt);
	add_func("NixGetShaderData",					TypeVoid,	nix_p(&NixGetShaderData));
		func_add_param("index",		TypeInt);
		func_add_param("name",		TypeString);
		func_add_param("data",		TypePointer);
		func_add_param("size",		TypeInt);
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
	add_ext_var("Api",				TypeString,		nix_p(&NixApi));
	add_ext_var("MinDepth",			TypeFloat,		nix_p(&NixMinDepth));
	add_ext_var("MaxDepth",			TypeFloat,		nix_p(&NixMaxDepth));
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
	// nix perspetive options
	add_const("PerspectiveCenterSet",         TypeInt, nix_p(PerspectiveCenterSet));
	add_const("PerspectiveCenterAutoTarget",  TypeInt, nix_p(PerspectiveCenterAutoTarget));
	add_const("PerspectiveSizeSet",           TypeInt, nix_p(PerspectiveSizeSet));
	add_const("PerspectiveSizeAutoTarget",    TypeInt, nix_p(PerspectiveSizeAutoTarget));
	add_const("PerspectiveSizeAutoScreen",    TypeInt, nix_p(PerspectiveSizeAutoScreen));
	add_const("Perspective2DScaleSet",        TypeInt, nix_p(Perspective2DScaleSet));
	add_const("Perspective2DScaleAutoTarget", TypeInt, nix_p(Perspective2DScaleAutoTarget));
	add_const("Perspective2DScaleAutoScreen", TypeInt, nix_p(Perspective2DScaleAutoScreen));
	add_const("PerspectiveRatioSet",          TypeInt, nix_p(PerspectiveRatioSet));
#ifdef OS_WINDOWS
	add_const("OSWindows", TypeInt, (void*)1);
#else
	add_const("OSWindows", TypeInt, (void*)-1);
#endif
#ifdef OS_LINUX
	add_const("OSLinux", TypeInt, (void*)1);
#else
	add_const("OSLinux", TypeInt, (void*)-1);
#endif
	add_const("VBTemp",     TypeInt,    nix_p(VBTemp));
	add_const("NixVersion", TypeString, nix_p(&NixVersion));
}

};
