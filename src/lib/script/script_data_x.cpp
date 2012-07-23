#include "../file/file.h"
#include "script.h"
#include "../00_config.h"
#include "script_data_common.h"


#ifdef _X_ALLOW_META_
	#include "../x/x.h"
#endif
#ifdef _X_ALLOW_X_
	#include "../networking.h"
#endif

#ifdef _X_ALLOW_META_
	#define meta_p(p)	(void*)p
#else
	#define meta_p(p)	NULL
#endif
#ifdef _X_ALLOW_GOD_
	#define god_p(p)	(void*)p
#else
	#define god_p(p)	NULL
#endif
#ifdef _X_ALLOW_MODEL_
	#define mod_p(p)	(void*)p
#else
	#define mod_p(p)	NULL
#endif
#ifdef _X_ALLOW_FX_
	#define fx_p(p)	(void*)p
#else
	#define fx_p(p)	NULL
#endif
#ifdef _X_ALLOW_GUI_
	#define gui_p(p)	(void*)p
#else
	#define gui_p(p)	NULL
#endif
#ifdef _X_ALLOW_CAMERA_
	#define cam_p(p)	(void*)p
#else
	#define cam_p(p)	NULL
#endif
#ifdef _X_ALLOW_X_
	#define x_p(p)		(void*)p
#else
	#define x_p(p)		NULL
#endif

sType *TypeBone;
sType *TypeBoneList;
sType *TypeModel;
sType *TypeModelP;
sType *TypeModelPPs;
sType *TypeModelPList;
sType *TypeModelPListPs;
sType *TypeText;
sType *TypeTextP;
sType *TypePicture;
sType *TypePictureP;
sType *TypePicture3D;
sType *TypePicture3DP;
sType *TypeGrouping;
sType *TypeGroupingP;
sType *TypeParticle;
sType *TypeParticleP;
sType *TypeBeam;
sType *TypeBeamP;
sType *TypeEffect;
sType *TypeEffectP;
sType *TypeCamera;
sType *TypeCameraP;
sType *TypeSkin;
sType *TypeSkinP;
sType *TypeSkinPArray;
sType *TypeSubSkin;
sType *TypeSubSkinList;
sType *TypeMaterial;
sType *TypeMaterialList;
sType *TypeFog;
sType *TypeTerrain;
sType *TypeTerrainP;
sType *TypeTerrainPList;

extern sType *TypeMatrix;
extern sType *TypeMatrix3;
extern sType *TypeFloatList;
extern sType *TypeIntList;
extern sType *TypePointerList;
extern sType *TypeVectorList;
extern sType *TypeVectorArray;
extern sType *TypeIntArray;
extern sType *TypeFloatArray;
extern sType *TypeFloatPs;
extern sType *TypePlaneList;
extern sType *TypeStringList;
extern sType *TypePlane;
extern sType *TypePlaneList;


#ifdef _X_ALLOW_GUI_
	static sText *_text;
	#define	GetDAText(x)		long(&_text->x)-long(_text)
	static sPicture *_picture;
	#define	GetDAPicture(x)		long(&_picture->x)-long(_picture)
	static sPicture3D *_picture3d;
	#define	GetDAPicture3D(x)	long(&_picture3d->x)-long(_picture3d)
	static sGrouping *_grouping;
	#define	GetDAGrouping(x)	long(&_grouping->x)-long(_grouping)
#else
	#define	GetDAText(x)		0
	#define	GetDAPicture(x)		0
	#define	GetDAPicture3D(x)	0
	#define	GetDAGrouping(x)	0
#endif
#ifdef _X_ALLOW_FX_
	static sParticle *_particle;
	#define	GetDAParticle(x)	long(&_particle->x)-long(_particle)
	static sEffect *_effect;
	#define	GetDAEffect(x)		long(&_effect->x)-long(_effect)
#else
	#define	GetDAParticle(x)	0
	#define	GetDABeam(x)		0
	#define	GetDAEffect(x)		0
#endif
#ifdef _X_ALLOW_MODEL_
	static Bone *_bone;
	#define	GetDABone(x)		long(&_bone->x)-long(_bone)
	static CModel *_model;
	#define sizeof_Bone			sizeof(Bone)
	#define	GetDAModel(x)		long(&_model->x)-long(_model)
	static Skin *_skin;
	#define	GetDASkin(x)		long(&_skin->x)-long(_skin)
	static SubSkin *_subskin;
	#define	GetDASubSkin(x)		long(&_subskin->x)-long(_subskin)
	static Material *_material;
	#define	GetDAMaterial(x)	long(&_material->x)-long(_material)
#else
	#define	GetDAItem(x)		0
	#define	GetDABone(x)		0
	#define sizeof_Bone			0
	#define	GetDAModel(x)		0
	#define	GetDASkin(x)		0
	#define	GetDASubSkin(x)		0
	#define	GetDAMaterial(x)	0
#endif
#ifdef _X_ALLOW_GOD_
	static Fog *_fog;
	#define	GetDAFog(x)			long(&_fog->x)-long(_fog)
#else
	#define	GetDAFog(x)			0
#endif
#ifdef _X_ALLOW_CAMERA_
	static Camera *_camera;
	#define	GetDACamera(x)		long(&_camera->x)-long(_camera)
#else
	#define	GetDACamera(x)		0
#endif
#ifdef _X_ALLOW_TERRAIN_
	static CTerrain *_terrain;
	#define	GetDATerrain(x)		long(&_terrain->x)-long(_terrain)
#else
	#define	GetDATerrain(x)		0
#endif

void SIAddPackageX()
{
	set_cur_package("x");

	
	TypeModel			= add_type  ("Model",		0);
	TypeModelP			= add_type_p("model",		TypeModel);
	TypeModelPPs		= add_type_p("model&",		TypeModelP);
	TypeModelPList		= add_type_a("model[]",		TypeModelP, -1);
	TypeModelPListPs	= add_type_p("model[]&",	TypeModelPList, FLAG_SILENT);
	TypeBone			= add_type  ("Bone",		sizeof_Bone);
	TypeBoneList		= add_type_a("Bone[]",		TypeBone, -1);
	TypeText			= add_type  ("Text",		0);
	TypeTextP			= add_type_p("text",		TypeText);
	TypePicture			= add_type  ("Picture",		0);
	TypePictureP		= add_type_p("picture",		TypePicture);
	TypePicture3D		= add_type  ("Picture3d",	0);
	TypePicture3DP		= add_type_p("picture3d",	TypePicture3D);
	TypeGrouping		= add_type  ("Grouping",	0);
	TypeGroupingP		= add_type_p("grouping",	TypeGrouping);
	TypeParticle		= add_type  ("Particle",	0);
	TypeParticleP		= add_type_p("particle",	TypeParticle);
	TypeBeam			= add_type  ("Beam",		0);
	TypeBeamP			= add_type_p("beam",		TypeBeam);
	TypeEffect			= add_type  ("Effect",		0);
	TypeEffectP			= add_type_p("effect",		TypeEffect);
	TypeCamera			= add_type  ("Camera",		0);
	TypeCameraP			= add_type_p("camera",		TypeCamera);
	TypeSkin			= add_type  ("Skin",		0);
	TypeSkinP			= add_type_p("skin",		TypeSkin);
	TypeSkinPArray		= add_type_a("skin[?]",		TypeSkinP, 1);
	TypeSubSkin			= add_type  ("SubSkin",		0);
	TypeSubSkinList		= add_type_a("SubSkin[]",	TypeSubSkin, -1);
	TypeMaterial		= add_type  ("Material",	0);
	TypeMaterialList	= add_type_a("Material[]",	TypeMaterial, -1);
	TypeFog				= add_type  ("Fog",			0);
	TypeTerrain			= add_type  ("Terrain",		0);
	TypeTerrainP		= add_type_p("terrain",		TypeTerrain);
	TypeTerrainPList	= add_type_a("terrain[]",	TypeTerrainP, -1);
	

	// bone, subskin, material...

	add_class(TypePicture);
		class_add_element("enabled",		TypeBool,		GetDAPicture(enabled));
		class_add_element("tc_inverted",	TypeBool,		GetDAPicture(tc_inverted));
		class_add_element("pos",			TypeVector,		GetDAPicture(pos));
		class_add_element("width",			TypeFloat,		GetDAPicture(width));
		class_add_element("height",			TypeFloat,		GetDAPicture(height));
		class_add_element("color",			TypeColor,		GetDAPicture(_color));
		class_add_element("texture",		TypeInt,		GetDAPicture(texture));
		class_add_element("source",			TypeRect,		GetDAPicture(source));
		class_add_element("shader",			TypeInt,		GetDAPicture(shader));
		class_add_func("IsMouseOver",		TypeBool,	gui_p(mf((tmf)&sPicture::IsMouseOver)));
	
	add_class(TypePicture3D);
		class_add_element("enabled",		TypeBool,		GetDAPicture3D(enabled));
		class_add_element("lighting",		TypeBool,		GetDAPicture3D(lighting));
		class_add_element("world_3d",		TypeBool,		GetDAPicture3D(world_3d));
		class_add_element("z",				TypeFloat,		GetDAPicture3D(z));
		class_add_element("matrix",			TypeMatrix,		GetDAPicture3D(_matrix));
		class_add_element("model",			TypeModelP,		GetDAPicture3D(model));
	
	add_class(TypeGrouping);
		class_add_element("enabled",		TypeBool,		GetDAGrouping(enabled));
		class_add_element("pos",			TypeVector,		GetDAGrouping(pos));
		class_add_element("color",			TypeColor,		GetDAGrouping(_color));
	
	add_class(TypeText);
		class_add_element("enabled",		TypeBool,		GetDAText(enabled));
		class_add_element("centric",		TypeBool,		GetDAText(centric));
		class_add_element("vertical",		TypeBool,		GetDAText(vertical));
		class_add_element("font",			TypeInt,		GetDAText(font));
		class_add_element("pos",			TypeVector,		GetDAText(pos));
		class_add_element("size",			TypeFloat,		GetDAText(size));
		class_add_element("color",			TypeColor,		GetDAText(_color));
		class_add_element("text",			TypeString,		GetDAText(text));
		class_add_func("IsMouseOver",		TypeBool,	gui_p(mf((tmf)&sText::IsMouseOver)));
	
	add_class(TypeParticle);
		class_add_element("enabled",		TypeBool,		GetDAParticle(enabled));
		class_add_element("suicidal",		TypeBool,		GetDAParticle(suicidal));
		class_add_element("pos",			TypeVector,		GetDAParticle(pos));
		class_add_element("vel",			TypeVector,		GetDAParticle(vel));
		class_add_element("ang",			TypeVector,		GetDAParticle(parameter));
		class_add_element("time_to_live",	TypeFloat,		GetDAParticle(time_to_live));
		class_add_element("radius",			TypeFloat,		GetDAParticle(radius));
		class_add_element("color",			TypeColor,		GetDAParticle(_color));
		class_add_element("texture",		TypeInt,		GetDAParticle(texture));
		class_add_element("source",			TypeRect,		GetDAParticle(source));
		class_add_element("func_delta_t",	TypeFloat,		GetDAParticle(func_delta_t));
		class_add_element("elapsed",		TypeFloat,		GetDAParticle(elapsed));
		class_add_element("func",			TypePointer,	GetDAParticle(func));

	add_class(TypeBeam);
		class_add_element("enabled",		TypeBool,		GetDAParticle(enabled));
		class_add_element("suicidal",		TypeBool,		GetDAParticle(suicidal));
		class_add_element("pos",			TypeVector,		GetDAParticle(pos));
		class_add_element("vel",			TypeVector,		GetDAParticle(vel));
		class_add_element("length",			TypeVector,		GetDAParticle(parameter));
		class_add_element("time_to_live",	TypeFloat,		GetDAParticle(time_to_live));
		class_add_element("radius",			TypeFloat,		GetDAParticle(radius));
		class_add_element("color",			TypeColor,		GetDAParticle(_color));
		class_add_element("texture",		TypeInt,		GetDAParticle(texture));
		class_add_element("source",			TypeRect,		GetDAParticle(source));
		class_add_element("func_delta_t",	TypeFloat,		GetDAParticle(func_delta_t));
		class_add_element("elapsed",		TypeFloat,		GetDAParticle(elapsed));
		class_add_element("func",			TypePointer,	GetDAParticle(func));
	
	add_class(TypeEffect);
		class_add_element("enabled",		TypeBool,		GetDAEffect(enabled));
		class_add_element("used",			TypeBool,		GetDAEffect(used));
		class_add_element("suicidal",		TypeBool,		GetDAEffect(suicidal));
		class_add_element("pos",			TypeVector,		GetDAEffect(pos));
		class_add_element("vel",			TypeVector,		GetDAEffect(vel));
		class_add_element("time_to_live",	TypeFloat,		GetDAEffect(time_to_live));
		class_add_element("var",			TypeFloatList,	GetDAEffect(script_var));
		class_add_element("var_i",			TypeIntList,	GetDAEffect(script_var));
		class_add_element("var_p",			TypePointerList,GetDAEffect(script_var));
		class_add_element("func_delta_t",	TypeFloat,		GetDAEffect(func_delta_t));
		class_add_element("elapsed",		TypeFloat,		GetDAEffect(elapsed));
		class_add_element("func",			TypePointer,	GetDAEffect(func));
		class_add_element("del_func",		TypePointer,	GetDAEffect(func_del));
		class_add_element("model",			TypeModelP,		GetDAEffect(model));
		class_add_element("vertex",			TypeInt,		GetDAEffect(vertex));

	add_class(TypeSkin);
		class_add_element("vertex",			TypeVectorList,	GetDASkin(vertex));
		class_add_element("bone_index",		TypeIntList,	GetDASkin(bone_index));
		class_add_element("sub",			TypeSubSkinList,GetDASkin(sub));

	add_class(TypeSubSkin);
		class_add_element("num_triangles",	TypeInt,		GetDASubSkin(num_triangles));
		class_add_element("triangle_index",	TypeIntList,	GetDASubSkin(triangle_index));
		class_add_element("skin_vertex",	TypeFloatList,	GetDASubSkin(skin_vertex));
		class_add_element("normal",			TypeVectorList,	GetDASubSkin(normal));

	add_class(TypeMaterial);
		class_add_element("num_textures",	TypeInt,		GetDAMaterial(num_textures));
		class_add_element("texture",		TypeIntArray,	GetDAMaterial(texture));
		class_add_element("shader",			TypeInt,		GetDAMaterial(shader));
		class_add_element("alpha_factor",	TypeFloat,		GetDAMaterial(alpha_factor));
		class_add_element("ambient",		TypeColor,		GetDAMaterial(ambient));
		class_add_element("diffuse",		TypeColor,		GetDAMaterial(diffuse));
		class_add_element("specular",		TypeColor,		GetDAMaterial(specular));
		class_add_element("emission",		TypeColor,		GetDAMaterial(emission));
		class_add_element("shininess",		TypeFloat,		GetDAMaterial(shininess));

	add_class(TypeFog);
		class_add_element("enabled",		TypeBool,		GetDAFog(enabled));
		class_add_element("mode",			TypeInt,		GetDAFog(mode));
		class_add_element("start",			TypeFloat,		GetDAFog(start));
		class_add_element("end",			TypeFloat,		GetDAFog(end));
		class_add_element("density",		TypeFloat,		GetDAFog(density));
		class_add_element("color",			TypeColor,		GetDAFog(_color));


	add_class(TypeBone);
		class_add_element("parent",		TypeInt,		GetDABone(parent));
		class_add_element("pos",		TypeVector,		GetDABone(pos));
		class_add_element("model",		TypeModelP,		GetDABone(model));
		class_add_element("dmatrix",	TypeMatrix,		GetDABone(dmatrix));
		class_add_element("cur_ang",		TypeQuaternion,	GetDABone(cur_ang));
		class_add_element("cur_pos",		TypeVector,		GetDABone(cur_pos));

	add_class(TypeModel);
		class_add_element("pos",			TypeVector,		GetDAModel(pos));
		class_add_element("vel",			TypeVector,		GetDAModel(vel));
		class_add_element("vel_surf",		TypeVector,		GetDAModel(vel_surf));
		class_add_element("ang",			TypeVector,		GetDAModel(ang));
		class_add_element("rot",			TypeVector,		GetDAModel(rot));
		class_add_element("name",			TypeString,		GetDAModel(name));
		class_add_element("description",	TypeString,		GetDAModel(description));
		class_add_element("on_ground",		TypeBool,		GetDAModel(on_ground));
		class_add_element("ground_id",		TypeInt,		GetDAModel(ground_id));
		class_add_element("ground_normal",	TypeVector,		GetDAModel(ground_normal));
		class_add_element("g_factor",		TypeFloat,		GetDAModel(g_factor));
		class_add_element("object_id",		TypeInt,		GetDAModel(object_id));
		//class_add_element("visible",		TypeBool,		GetDAModel(visible));
		class_add_element("active_physics",	TypeBool,		GetDAModel(active_physics));
		class_add_element("passive_physics",	TypeBool,		GetDAModel(passive_physics));
		class_add_element("mass",			TypeFloat,		GetDAModel(mass));
		class_add_element("theta_0",		TypeMatrix3,	GetDAModel(theta_0));
		class_add_element("theta",			TypeMatrix3,	GetDAModel(theta));
		class_add_element("matrix",			TypeMatrix,		GetDAModel(_matrix));
		class_add_element("radius",			TypeFloat,		GetDAModel(radius));
		class_add_element("detail_dist",	TypeFloatArray,	GetDAModel(detail_dist));
		class_add_element("var",			TypeFloatList,	GetDAModel(script_var));
		class_add_element("var_i",			TypeIntList,	GetDAModel(script_var));
		class_add_element("var_p",			TypePointerList,GetDAModel(script_var));
		class_add_element("data",			TypePointer,	GetDAModel(script_data));
		class_add_element("item",			TypeModelPList,	GetDAModel(inventary));
		class_add_func("AddForce",		TypeVoid,	god_p(mf((tmf)&CObject::AddForce)));
			func_add_param("force",		TypeVector);
			func_add_param("rho",		TypeVector);
		class_add_func("AddTorque",		TypeVoid,	god_p(mf((tmf)&CObject::AddTorque)));
			func_add_param("torque",		TypeVector);
		class_add_func("MakeVisible",		TypeVoid,		god_p(mf((tmf)&CObject::MakeVisible)));
			func_add_param("visible",		TypeBool);
		class_add_func("UpdateData",		TypeVoid,		god_p(mf((tmf)&CObject::UpdateData)));
		class_add_element("skin",			TypeSkinPArray,	GetDAModel(skin));
		class_add_element("skin0",			TypeSkinP,		GetDAModel(skin[0]));
		class_add_element("skin1",			TypeSkinP,		GetDAModel(skin[1]));
		class_add_element("skin2",			TypeSkinP,		GetDAModel(skin[2]));
		class_add_element("material",		TypeMaterialList,	GetDAModel(material));
		class_add_element("bone",			TypeBoneList,	GetDAModel(bone));
		class_add_element("min",			TypeVector,		GetDAModel(min));
		class_add_element("max",			TypeVector,		GetDAModel(max));
		class_add_element("test_collisions",	TypeBool,		GetDAModel(test_collisions));
		class_add_element("allow_shadow",	TypeBool,		GetDAModel(allow_shadow));
/*	add_func("CalcMove",					TypeVoid,		mod_p(mf((tmf)&CModel::CalcMove)));
	add_func("Draw",						TypeVoid,		mod_p(mf((tmf)&CModel::Draw)));
		func_add_param("skin",				TypeInt);
		func_add_param("fx",				TypeBool);*/
		class_add_func("GetVertex",		TypeVector,		mod_p(mf((tmf)&CModel::GetVertex)));
			func_add_param("index",			TypeInt);
			func_add_param("skin",			TypeInt);
		class_add_func("ResetAnimation",		TypeVoid,		mod_p(mf((tmf)&CModel::ResetAnimation)));
		class_add_func("Animate",				TypeBool,		mod_p(mf((tmf)&CModel::Animate)));
			func_add_param("operation",		TypeInt);
			func_add_param("param1",		TypeFloat);
			func_add_param("param2",		TypeFloat);
			func_add_param("move",			TypeInt);
			func_add_param("time",			TypeFloatPs);
			func_add_param("dt",			TypeFloat);
			func_add_param("v",				TypeFloat);
			func_add_param("loop",			TypeBool);
		class_add_func("GetFrames",		TypeInt,		mod_p(mf((tmf)&CModel::GetFrames)));
			func_add_param("move",			TypeInt);
		class_add_func("BeginEditAnimation",	TypeVoid,		mod_p(mf((tmf)&CModel::BeginEditAnimation)));
		class_add_func("MakeEditable",		TypeVoid,		mod_p(mf((tmf)&CModel::MakeEditable)));
		class_add_func("BeginEdit",		TypeVoid,		mod_p(mf((tmf)&CModel::BeginEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("EndEdit",			TypeVoid,		mod_p(mf((tmf)&CModel::EndEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("SetBoneModel",		TypeVoid,		mod_p(mf((tmf)&CModel::SetBoneModel)));
			func_add_param("index",			TypeInt);
			func_add_param("bone",			TypeModelP);
		class_add_func("GetFilename",		TypeString,		mod_p(mf((tmf)&CModel::GetFilename)));

	add_class(TypeTerrain);
		class_add_element("pos",			TypeVector,		GetDATerrain(pos));
		class_add_element("num_x",			TypeInt,		GetDATerrain(num_x));
		class_add_element("num_z",			TypeInt,		GetDATerrain(num_z));
		class_add_element("height",			TypeFloatList,	GetDATerrain(height));
		class_add_element("pattern",		TypeVector,		GetDATerrain(pattern));
		class_add_element("num_textures",	TypeInt,		GetDATerrain(num_textures));
		class_add_element("texture",		TypeIntArray,	GetDATerrain(texture));
		class_add_element("texture_scale",	TypeVectorArray,GetDATerrain(texture_scale));
		class_add_func("Update",			TypeVoid,		god_p(mf((tmf)&CTerrain::Update)));
			func_add_param("x1",		TypeInt);
			func_add_param("x2",		TypeInt);
			func_add_param("z1",		TypeInt);
			func_add_param("z2",		TypeInt);
			func_add_param("mode",		TypeInt);
		class_add_func("GetHeight",			TypeFloat,		god_p(mf((tmf)&CTerrain::gimme_height)));
			func_add_param("p",			TypeVector);

	add_class(TypeCamera);
		class_add_element("enabled",		TypeBool,		GetDACamera(enabled));
		class_add_element("show",			TypeBool,		GetDACamera(show));
		class_add_element("texture_out",	TypeInt,		GetDACamera(output_texture));
		class_add_element("texture_in",	TypeInt,		GetDACamera(input_texture));
		class_add_element("shader",			TypeInt,		GetDACamera(shader));
		class_add_element("shaded_displays",TypeBool,		GetDACamera(shaded_displays));
		class_add_element("pos",			TypeVector,		GetDACamera(pos));
		class_add_element("ang",			TypeVector,		GetDACamera(ang));
		class_add_element("vel",			TypeVector,		GetDACamera(vel));
		class_add_element("rot",			TypeVector,		GetDACamera(rot));
		class_add_element("zoom",			TypeFloat,		GetDACamera(zoom));
		class_add_element("dest",			TypeRect,		GetDACamera(dest));
		class_add_element("z",				TypeFloat,		GetDACamera(z));
		class_add_element("min_depth",		TypeFloat,		GetDACamera(min_depth));
		class_add_element("max_depth",		TypeFloat,		GetDACamera(max_depth));
		class_add_element("clipping_plane",	TypePlaneList,	GetDACamera(clipping_plane));
		class_add_element("ignore",			TypeModelPList,	GetDACamera(ignore));
		class_add_func("StartScript",		TypeVoid,	cam_p(mf((tmf)&Camera::StartScript)));
			func_add_param("filename",		TypeString);
			func_add_param("dpos",			TypeVector);
		class_add_func("StopScript",		TypeVoid,	cam_p(mf((tmf)&Camera::StopScript)));
		class_add_func("Project",		TypeVector,	cam_p(mf((tmf)&Camera::Project)));
			func_add_param("v",			TypeVector);
		class_add_func("Unproject",		TypeVector,	cam_p(mf((tmf)&Camera::Unproject)));
			func_add_param("v",			TypeVector);

	

	
	add_func("XFDrawStr",			TypeFloat,	meta_p(&XFDrawStr));
		func_add_param("x",			TypeFloat);
		func_add_param("y",			TypeFloat);
		func_add_param("size",		TypeFloat);
		func_add_param("str",		TypeString);
		func_add_param("centric",	TypeBool);
	add_func("XFDrawVertStr",		TypeFloat,	meta_p(&XFDrawVertStr));
		func_add_param("x",			TypeFloat);
		func_add_param("y",			TypeFloat);
		func_add_param("size",		TypeFloat);
		func_add_param("str",		TypeString);
	add_func("XFGetWidth",			TypeFloat,	meta_p(&XFGetWidth));
		func_add_param("size",		TypeFloat);
		func_add_param("s",		TypeString);
	add_func("LoadFont",			TypeInt,	meta_p(&MetaLoadXFont));
		func_add_param("filename",		TypeString);
	add_func("CreatePicture",										TypePictureP,	gui_p(&GuiCreatePicture));
		func_add_param("pos",		TypeVector);
		func_add_param("width",		TypeFloat);
		func_add_param("height",	TypeFloat);
		func_add_param("texture",	TypeInt);
	add_func("CreatePicture3D",								TypePicture3DP,	gui_p(&GuiCreatePicture3D));
		func_add_param("m",			TypeModelP);
		func_add_param("mat",		TypeMatrix);
		func_add_param("z",			TypeFloat);
	add_func("CreateText",												TypeTextP,	gui_p(&GuiCreateText));
		func_add_param("pos",		TypeVector);
		func_add_param("size",		TypeFloat);
		func_add_param("c",			TypeColor);
		func_add_param("str",		TypeString);
	add_func("CreateGrouping",										TypeGroupingP,	gui_p(&GuiCreateGrouping));
		func_add_param("pos",		TypeVector);
		func_add_param("set_cur",	TypeBool);
	add_func("CreateParticle",										TypeParticleP,	fx_p(&FxParticleCreateDef));
		func_add_param("pos",		TypeVector);
		func_add_param("texture",		TypeInt);
		func_add_param("func",		TypePointer);
		func_add_param("life",		TypeFloat);
		func_add_param("radius",		TypeFloat);
	add_func("CreateParticleRot",								TypeParticleP,	fx_p(&FxParticleCreateRot));
		func_add_param("pos",		TypeVector);
		func_add_param("ang",		TypeVector);
		func_add_param("texture",		TypeInt);
		func_add_param("func",		TypePointer);
		func_add_param("life",		TypeFloat);
		func_add_param("radius",		TypeFloat);
	add_func("CreateBeam",										TypeBeamP,	fx_p(&FxParticleCreateBeam));
		func_add_param("pos",		TypeVector);
		func_add_param("length",		TypeVector);
		func_add_param("texture",		TypeInt);
		func_add_param("func",		TypePointer);
		func_add_param("life",		TypeFloat);
		func_add_param("radius",		TypeFloat);
	add_func("CreateEffect",										TypeEffectP,	fx_p(&FxCreate));
		func_add_param("pos",		TypeVector);
		func_add_param("func",		TypePointer);
		func_add_param("del_func",		TypePointer);
		func_add_param("life",		TypeFloat);
	add_func("LoadModel",												TypeModelP,	meta_p(&MetaLoadModel));
		func_add_param("filename",		TypeString);
/*	add_func("GetModelOID",												TypeInt,	meta_p(&MetaGetModelOID));
		func_add_param("filename",		TypeString);*/
	add_func("CreateCamera",												TypeCameraP,	cam_p(&CreateCamera));
		func_add_param("pos",		TypeVector);
		func_add_param("ang",		TypeVector);
		func_add_param("dest",		TypeRect);
		func_add_param("show",		TypeBool);
	add_func("LoadShader",										TypeInt,	meta_p(&MetaLoadShader));
		func_add_param("filename",		TypeString);
	
	// engine
	// effects
	add_func("LightCreate",							TypeInt,	fx_p(&FxLightCreate));
	add_func("LightSetDirectional",			TypeVoid,	fx_p(&FxLightSetDirectional));
		func_add_param("index",		TypeInt);
		func_add_param("dir",		TypeVector);
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
	add_func("LightSetRadial",					TypeVoid,	fx_p(&FxLightSetRadial));
		func_add_param("index",		TypeInt);
		func_add_param("pos",		TypeVector);
		func_add_param("radius",		TypeFloat);
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
	add_func("LightEnable",							TypeVoid,	fx_p(&FxLightEnable));
		func_add_param("index",		TypeInt);
		func_add_param("enabled",		TypeBool);
	add_func("LightDelete",							TypeVoid,	fx_p(&FxLightDelete));
		func_add_param("index",		TypeInt);
	// game
	add_func("ExitProgram",									TypeVoid,	meta_p(MetaExitProgram));
	add_func("ScreenShot",									TypeVoid,	meta_p(MetaScreenShot));
	add_func("FindHosts",									TypeVoid,	meta_p(MetaFindHosts));
	add_func("XDelete",											TypeVoid,	meta_p(&MetaDelete));
		func_add_param("p",		TypePointer);
	add_func("XDeleteLater",						TypeVoid,	god_p(&MetaDeleteLater));
		func_add_param("p",		TypePointer);
	add_func("XDeleteSelection",						TypeVoid,	god_p(&MetaDeleteSelection));
	add_func("LoadWorld",									TypeVoid,	meta_p(MetaLoadWorld));
		func_add_param("filename",		TypeString);
	add_func("LoadGameFromHost",					TypeVoid,	meta_p(MetaLoadGameFromHost));
		func_add_param("host",		TypeInt);
	add_func("SaveGameState",							TypeVoid,	meta_p(MetaSaveGameState));
		func_add_param("filename",		TypeString);
	add_func("LoadGameState",							TypeVoid,	meta_p(MetaLoadGameState));
		func_add_param("filename",		TypeString);
	add_func("GetObjectByName",							TypeModelP,	god_p(&GetObjectByName));
		func_add_param("name",		TypeString);
	add_func("FindObjects",								TypeInt,	god_p(&GodFindObjects));
		func_add_param("pos",		TypeVector);
		func_add_param("radius",	TypeFloat);
		func_add_param("mode",		TypeInt);
		func_add_param("o",			TypeModelPListPs);
	add_func("NextObject",									TypeBool,	god_p(&NextObject));
		func_add_param("o",		TypeModelPPs);
	add_func("CreateObject",							TypeModelP,	god_p(&_CreateObject));
		func_add_param("filename",		TypeString);
		func_add_param("pos",		TypeVector);
	add_func("SplashScreen",					TypeVoid,	meta_p(MetaDrawSplashScreen));
		func_add_param("status",		TypeString);
		func_add_param("progress",		TypeFloat);
	add_func("RenderScene",									TypeVoid, 	NULL);
	add_func("GetG",											TypeVector,	god_p(&GetG));
		func_add_param("pos",		TypeVector);
	add_func("Trace",											TypeBool,	god_p(&GodTrace));
		func_add_param("p1",		TypeVector);
		func_add_param("p2",		TypeVector);
		func_add_param("tp",		TypeVector);
		func_add_param("simple_test",	TypeBool);
		func_add_param("o_ignore",		TypeInt);
	add_func("LinkAddSpring",							TypeInt,	god_p(&AddLinkSpring));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p1",		TypeVector);
		func_add_param("p2",		TypeVector);
		func_add_param("dx0",		TypeFloat);
		func_add_param("k",			TypeFloat);
	add_func("LinkAddBall",									TypeInt,	god_p(&AddLinkBall));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p",			TypeVector);
	add_func("LinkAddHinge",							TypeInt,	god_p(&AddLinkHinge));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p"	,		TypeVector);
		func_add_param("ax",		TypeVector);
	add_func("LinkAddHinge2",							TypeInt,	god_p(&AddLinkHinge2));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p"	,		TypeVector);
		func_add_param("ax1",		TypeVector);
		func_add_param("ax2",		TypeVector);
	add_func("LinkAddSlider",									TypeInt,	god_p(&AddLinkSlider));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("ax",		TypeVector);
	add_func("LinkAddUniversal",									TypeInt,	god_p(&AddLinkUniversal));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p",			TypeVector);
		func_add_param("ax1",		TypeVector);
		func_add_param("ax2",		TypeVector);
	add_func("LinkSetTorque",					TypeVoid,	god_p(&LinkSetTorque));
		func_add_param("link",		TypeInt);
		func_add_param("torque",	TypeFloat);
	add_func("LinkSetTorqueAxis",					TypeVoid,	god_p(&LinkSetTorqueAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);
		func_add_param("torque",	TypeFloat);
	add_func("LinkSetRange",					TypeVoid,	god_p(&LinkSetRange));
		func_add_param("link",		TypeInt);
		func_add_param("min",		TypeFloat);
		func_add_param("max",		TypeFloat);
	add_func("LinkSetRangeAxis",					TypeVoid,	god_p(&LinkSetRangeAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);
		func_add_param("min",		TypeFloat);
		func_add_param("max",		TypeFloat);
	add_func("LinkSetFriction",		TypeVoid,	god_p(&LinkSetFriction));
		func_add_param("link",		TypeInt);
		func_add_param("friction",	TypeFloat);
	/*add_func("LinkSetFrictionAxis",		TypeVoid,	god_p(&LinkSetFrictionAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);
		func_add_param("friction",	TypeFloat);*/
	add_func("LinkGetPosition",					TypeFloat,	god_p(&LinkGetPosition));
		func_add_param("link",		TypeInt);
	add_func("LinkGetPositionAxis",					TypeFloat,	god_p(&LinkGetPositionAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);

	

	// game variables
	add_ext_var("AppName",			TypeString,		god_p(&AppName));
	add_ext_var("Elapsed",			TypeFloat,		meta_p(&Elapsed));
	add_ext_var("ElapsedRT",		TypeFloat,		meta_p(&ElapsedRT));
	add_ext_var("TimeScale",		TypeFloat,		meta_p(&TimeScale));
	add_ext_var("InitialWorldFile", TypeString,		god_p(&InitialWorldFile));
	add_ext_var("CurrentWorldFile", TypeString,		god_p(&CurrentWorldFile));
	add_ext_var("SecondWorldFile",	TypeString,		god_p(&SecondWorldFile));
	add_ext_var("Object",			TypeModelPList,	god_p(&Object));
	add_ext_var("Ego",				TypeModelP,		god_p(&Ego));
	add_ext_var("Terrain",			TypeTerrainPList,god_p(&Terrain));
	add_ext_var("Gravitation",		TypeVector,		god_p(&GlobalG));
	add_ext_var("PhysicsEnabled",	TypeBool,		god_p(&PhysicsEnabled));
	add_ext_var("CollisionsEnabled",	TypeBool,		god_p(&CollisionsEnabled));
	add_ext_var("Cam",				TypeCameraP,		cam_p(&Cam));
	add_ext_var("SkyBox",			TypeModelPList,	god_p(&SkyBox));
	add_ext_var("BackGroundColor",	TypeColor,		god_p(&BackGroundColor));
	add_ext_var("GlobalFog",		TypeFog,		god_p(&GlobalFog));
	add_ext_var("ScriptVar",		TypeFloatList,	god_p(&ScriptVar));
	add_ext_var("Ambient",			TypeColor,		god_p(&GlobalAmbient));
	add_ext_var("SunLight",			TypeInt,		god_p(&SunLight));
	add_ext_var("SpeedOfSound",		TypeFloat,		god_p(&SpeedOfSound));
	add_ext_var("TraceHitType",		TypeInt,		god_p(&TraceHitType));
	add_ext_var("TraceHitIndex",	TypeInt,		god_p(&TraceHitIndex));
	add_ext_var("TraceHitSubModel", TypeInt,		god_p(&TraceHitSubModel));
	add_ext_var("CurrentGrouping",	TypeGroupingP,	gui_p(&CurrentGrouping));
	add_ext_var("MirrorLevelMax",	TypeInt,		god_p(&MirrorLevelMax));
	add_ext_var("SessionName",		TypeString,		x_p(&SessionName));
	add_ext_var("HostNames",		TypeString,		x_p(&HostNames));
	add_ext_var("AvailableHostName",TypeStringList,	x_p(&AvailableHostName));
	add_ext_var("AvailableSessionName",	TypeStringList,x_p(&AvailableSessionName));
	add_ext_var("NetIAmHost",		TypeBool,		x_p(&NetIAmHost));
	add_ext_var("NetIAmClient",		 TypeBool,		x_p(&NetIAmClient));
	add_ext_var("NetHasRead",		TypeBool,		x_p(&NetHasRead));
	add_ext_var("NetHasWritten",	TypeBool,		x_p(&NetHasWritten));
	add_ext_var("NumClients",		TypeInt,		x_p(&NumClients));
	add_ext_var("SocketToHost",		TypeInt,		x_p(&SocketToHost));
	add_ext_var("SocketToClient",	TypeIntList,	x_p(&SocketToClient));
	add_ext_var("CurrentSocket",	TypeInt,		x_p(&CurrentSocket));
	add_ext_var("Debug",			TypeBool,		meta_p(&Debug));
	add_ext_var("ShowTimings",		TypeBool,		meta_p(&ShowTimings));
	add_ext_var("WireMode",			TypeBool,		meta_p(&WireMode));
	add_ext_var("ConsoleEnabled",	TypeBool,		meta_p(&ConsoleEnabled));
	add_ext_var("Record",			TypeBool,		meta_p(&Record));
	add_ext_var("ShadowLevel",		TypeInt,		meta_p(&ShadowLevel));
	add_ext_var("ShadowLowerDetail",TypeBool,		meta_p(&ShadowLowerDetail));
	add_ext_var("ShadowLight",		TypeInt,		meta_p(&ShadowLight));
	add_ext_var("ShadowColor",		TypeColor,		meta_p(&ShadowColor));
	add_ext_var("FpsMax",			TypeFloat,		meta_p(&FpsMax));
	add_ext_var("FpsMin",			TypeFloat,		meta_p(&FpsMin));
	add_ext_var("Multisampling",	TypeInt,		meta_p(&Multisampling));
	add_ext_var("DetailLevel",		TypeInt,		meta_p(&DetailLevel));
	add_ext_var("DetailFactorInv",	TypeFloat,		meta_p(&DetailFactorInv));
	add_ext_var("NetworkEnabled",	TypeBool,		meta_p(&NetworkEnabled));
	add_ext_var("XFontIndex",		TypeInt,		meta_p(&XFontIndex));
	add_ext_var("DefaultFont",		TypeInt,		meta_p(&DefaultFont));
	add_ext_var("ResettingGame",	TypeBool,		meta_p(&ResettingGame));

	
	// model skins
	add_const("SkinHigh",   TypeInt, god_p(SkinHigh));
	add_const("SkinMedium", TypeInt, god_p(SkinMedium));
	add_const("SkinLow",    TypeInt, god_p(SkinLow));
	// trace
	add_const("TraceHitTerrain", TypeInt, god_p(TraceHitTerrain));
	add_const("TraceHitObject",  TypeInt, god_p(TraceHitObject));
	// animation operations
	add_const("MoveOpSet",         TypeInt, god_p(MoveOpSet));
	add_const("MoveOpSetNewKeyed", TypeInt, god_p(MoveOpSetNewKeyed));
	add_const("MoveOpSetOldKeyed", TypeInt, god_p(MoveOpSetOldKeyed));
	add_const("MoveOpAdd1Factor",  TypeInt, god_p(MoveOpAdd1Factor));
	add_const("MoveOpMix1Factor",  TypeInt, god_p(MoveOpMix1Factor));
	add_const("MoveOpMix2Factor",  TypeInt, god_p(MoveOpMix2Factor));
	
}
