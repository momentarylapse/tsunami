#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"


#ifdef _X_ALLOW_X_
	#include "../../../world/world.h"
	#include "../../../world/object.h"
	#include "../../../world/model_manager.h"
	#include "../../../world/terrain.h"
	#include "../../../world/camera.h"
	#include "../../../physics/links.h"
	#include "../../../gui/font.h"
	#include "../../../gui/gui.h"
	#include "../../../fx/fx.h"
	#include "../../../fx/light.h"
	#include "../../../meta.h"
	#include "../../../networking.h"
	#define x_p(p)		(void*)p
	void ExitProgram();
	void ScreenShot();
	void LoadWorldSoon(const string &filename);
	void LoadGameFromHostSoon(const HostData &host);
	void SaveGameState(const string &filename);
	void LoadGameStateSoon(const string &filename);
	void DrawSplashScreen(const string &str, float per);
	using namespace Gui;
	using namespace Fx;
#else
	#define x_p(p)		NULL
#endif

namespace Script{

Type *TypeBone;
Type *TypeBoneList;
Type *TypeModel;
Type *TypeModelP;
Type *TypeModelPPs;
Type *TypeModelPList;
Type *TypeModelPListPs;
Type *TypeText;
Type *TypeTextP;
Type *TypePicture;
Type *TypePictureP;
Type *TypePicture3D;
Type *TypePicture3DP;
Type *TypeLayer;
Type *TypeLayerP;
Type *TypeParticle;
Type *TypeParticleP;
Type *TypeBeam;
Type *TypeBeamP;
Type *TypeEffect;
Type *TypeEffectP;
Type *TypeCamera;
Type *TypeCameraP;
Type *TypeSkin;
Type *TypeSkinP;
Type *TypeSkinPArray;
Type *TypeSubSkin;
Type *TypeSubSkinList;
Type *TypeMaterial;
Type *TypeMaterialP;
Type *TypeMaterialList;
Type *TypeFog;
Type *TypeTraceData;
Type *TypeTerrain;
Type *TypeTerrainP;
Type *TypeTerrainPList;
Type *TypeEngineData;
Type *TypeWorldData;
Type *TypeNetworkData;
Type *TypeHostData;
Type *TypeHostDataList;

extern Type *TypeMatrix;
extern Type *TypeMatrix3;
extern Type *TypeFloatList;
extern Type *TypeIntList;
extern Type *TypeVectorList;
extern Type *TypeVectorArray;
extern Type *TypeIntArray;
extern Type *TypeFloatArray;
extern Type *TypeFloatPs;
extern Type *TypePlaneList;
extern Type *TypeSocket;
extern Type *TypeSocketP;
extern Type *TypeSocketList;


#ifdef _X_ALLOW_X_
	static Text *_text;
	#define	GetDAText(x)		long(&_text->x)-long(_text)
	static Picture *_picture;
	#define	GetDAPicture(x)		long(&_picture->x)-long(_picture)
	static Picture3d *_picture3d;
	#define	GetDAPicture3D(x)	long(&_picture3d->x)-long(_picture3d)
	static Layer *_layer;
	#define	GetDALayer(x)	long(&_layer->x)-long(_layer)
	static Particle *_particle;
	#define	GetDAParticle(x)	long(&_particle->x)-long(_particle)
	static Effect *_effect;
	#define	GetDAEffect(x)		long(&_effect->x)-long(_effect)
	static Bone *_bone;
	#define	GetDABone(x)		long(&_bone->x)-long(_bone)
	static Model *_model;
	#define	GetDAModel(x)		long(&_model->x)-long(_model)
	static Skin *_skin;
	#define	GetDASkin(x)		long(&_skin->x)-long(_skin)
	static SubSkin *_subskin;
	#define	GetDASubSkin(x)		long(&_subskin->x)-long(_subskin)
	static Material *_material;
	#define	GetDAMaterial(x)	long(&_material->x)-long(_material)
	static Fog *_fog;
	static WorldData *_world_data;
	static EngineData *_engine_data;
	static NetworkData *_network_data;
	static HostData *_host_data;
	#define	GetDAFog(x)			long(&_fog->x)-long(_fog)
	#define	GetDAWorld(x)			long(&_world_data->x)-long(_world_data)
	#define	GetDAEngine(x)			long(&_engine_data->x)-long(_engine_data)
	#define	GetDANetwork(x)			long(&_network_data->x)-long(_network_data)
	#define	GetDAHostData(x)			long(&_host_data->x)-long(_host_data)
	class HostDataList : public Array<HostData>
	{
		public:
			void __init__(){Array<HostData>::__init__();}
			void __delete__(){clear();}
			void __assign__(HostDataList &other){*this = other;}
	};
	static Camera *_camera;
	#define	GetDACamera(x)		long(&_camera->x)-long(_camera)
	static Terrain *_terrain;
	#define	GetDATerrain(x)		long(&_terrain->x)-long(_terrain)
	static TraceData *_tracedata;
	#define	GetDATraceData(x)		long(&_tracedata->x)-long(_tracedata)
	#define class_set_vtable_x(x)	class_set_vtable(x)
#else
	typedef int Picture;
	typedef int Picture3d;
	typedef int Text;
	typedef int Layer;
	typedef int Particle;
	typedef int Effect;
	typedef int Camera;
	typedef int Fog;
	#define	GetDAText(x)		0
	#define	GetDAPicture(x)		0
	#define	GetDAPicture3D(x)	0
	#define	GetDALayer(x)		0
	#define	GetDAParticle(x)	0
	#define	GetDABeam(x)		0
	#define	GetDAEffect(x)		0
	#define	GetDAItem(x)		0
	#define	GetDABone(x)		0
	#define	GetDAModel(x)		0
	#define	GetDASkin(x)		0
	#define	GetDASubSkin(x)		0
	#define	GetDAMaterial(x)	0
	#define	GetDAFog(x)			0
	#define	GetDAWorld(x)		0
	#define	GetDAEngine(x)		0
	#define	GetDANetwork(x)		0
	#define	GetDAHostData(x)	0
	#define	GetDACamera(x)		0
	#define	GetDATerrain(x)		0
	#define	GetDATraceData(x)	0
	typedef int TraceData;
	typedef int Bone;
	#define class_set_vtable_x(x)
#endif


#ifdef _X_ALLOW_X_
void amd64_model_get_vertex(vector &r, Model *m, int a, int b)
{	r = m->GetVertex(a, b);	}
void amd64_camera_project(vector &r, Camera *c, vector &v)
{	r = c->Project(v);	}
void amd64_camera_unproject(vector &r, Camera *c, vector &v)
{	r = c->Unproject(v);	}
void amd64_getg(vector &r, vector &v)
{	r = GetG(v);	}
#define amd64_wrap(orig, wrap)	((config.instruction_set == Asm::InstructionSetAMD64) ? ((void*)(wrap)) : ((void*)(orig)))
#else
#define amd64_wrap(a, b)	NULL
#endif


void SIAddPackageX()
{
	add_package("x", false);
#if _X_ALLOW_X_
	AllowXContainer = false;
#endif

	
	TypeModel			= add_type  ("Model",		0);
	TypeModelP			= add_type_p("model",		TypeModel);
	TypeModelPPs		= add_type_p("model&",		TypeModelP);
	TypeModelPList		= add_type_a("model[]",		TypeModelP, -1);
	TypeModelPListPs	= add_type_p("model[]&",	TypeModelPList, FLAG_SILENT);
	TypeBone			= add_type  ("Bone",		sizeof(Bone));
	TypeBoneList		= add_type_a("Bone[]",		TypeBone, -1);
	TypeText			= add_type  ("Text",		sizeof(Text));
	TypeTextP			= add_type_p("text",		TypeText);
	TypePicture			= add_type  ("Picture",		sizeof(Picture));
	TypePictureP		= add_type_p("picture",		TypePicture);
	TypePicture3D		= add_type  ("Picture3d",	sizeof(Picture3d));
	TypePicture3DP		= add_type_p("picture3d",	TypePicture3D);
	TypeLayer			= add_type  ("Layer",		sizeof(Layer));
	TypeLayerP			= add_type_p("layer",		TypeLayer);
	TypeParticle		= add_type  ("Particle",	sizeof(Particle));
	TypeParticleP		= add_type_p("particle",	TypeParticle);
	TypeBeam			= add_type  ("Beam",		sizeof(Particle));
	TypeBeamP			= add_type_p("beam",		TypeBeam);
	TypeEffect			= add_type  ("Effect",		sizeof(Effect));
	TypeEffectP			= add_type_p("effect",		TypeEffect);
	TypeCamera			= add_type  ("Camera",		sizeof(Camera));
	TypeCameraP			= add_type_p("camera",		TypeCamera);
	TypeSkin			= add_type  ("Skin",		0);
	TypeSkinP			= add_type_p("skin",		TypeSkin);
	TypeSkinPArray		= add_type_a("skin[?]",		TypeSkinP, 1);
	TypeSubSkin			= add_type  ("SubSkin",		0);
	TypeSubSkinList		= add_type_a("SubSkin[]",	TypeSubSkin, -1);
	TypeMaterial		= add_type  ("Material",	0);
	TypeMaterialP		= add_type_p("Material*",	TypeMaterial);
	TypeMaterialList	= add_type_a("Material[]",	TypeMaterial, -1);
	TypeFog				= add_type  ("Fog",			sizeof(Fog));
	TypeTraceData		= add_type  ("TraceData",	sizeof(TraceData));
	TypeTerrain			= add_type  ("Terrain",		0);
	TypeTerrainP		= add_type_p("terrain",		TypeTerrain);
	TypeTerrainPList	= add_type_a("terrain[]",	TypeTerrainP, -1);
	TypeWorldData		= add_type  ("WorldData",	0);
	TypeEngineData		= add_type  ("EngineData",	0);
	TypeNetworkData		= add_type  ("NetworkData",	0);
	TypeHostData		= add_type  ("HostData",    0);
	TypeHostDataList	= add_type_a("HostData[]",	TypeHostData, -1);
	

	// bone, subskin, material...

	add_class(TypePicture);
		class_set_vtable_x(Picture);
		TypePicture->vtable = new VirtualTable[20];
		class_add_element("enabled",		TypeBool,		GetDAPicture(enabled));
		class_add_element("tc_inverted",	TypeBool,		GetDAPicture(tc_inverted));
		class_add_element("pos",			TypeVector,		GetDAPicture(pos));
		class_add_element("width",			TypeFloat,		GetDAPicture(width));
		class_add_element("height",			TypeFloat,		GetDAPicture(height));
		class_add_element("color",			TypeColor,		GetDAPicture(_color));
		class_add_element("texture",		TypeInt,		GetDAPicture(texture));
		class_add_element("source",			TypeRect,		GetDAPicture(source));
		class_add_element("shader",			TypeInt,		GetDAPicture(shader));
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Picture::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("width",		TypeFloat);
			func_add_param("height",	TypeFloat);
			func_add_param("texture",	TypeInt);
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Picture::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf((tmf)&Picture::__delete__)));
		class_add_func("__Draw", TypeVoid, x_p(mf((tmf)&Picture::Draw)));
		class_add_func("__Update", TypeVoid, x_p(mf((tmf)&Picture::Update)));
		class_add_func("OnIterate", TypeVoid, x_p(mf((tmf)&Picture::OnIterate)));
		class_add_func("OnHover", TypeVoid, x_p(mf((tmf)&Picture::OnHover)));
		class_add_func("OnClick", TypeVoid, x_p(mf((tmf)&Picture::OnClick)));
		class_add_func("OnMouseEnter", TypeVoid, x_p(mf((tmf)&Picture::OnMouseEnter)));
		class_add_func("OnMouseLeave", TypeVoid, x_p(mf((tmf)&Picture::OnMouseLeave)));
		class_add_func("IsMouseOver", TypeBool, x_p(mf((tmf)&Picture::IsMouseOver)));
		TypePicture->LinkVirtualTable();
	
	add_class(TypePicture3D);
		class_set_vtable_x(Picture3d);
		TypePicture3D->vtable = new VirtualTable[20];
		class_add_element("enabled",		TypeBool,		GetDAPicture3D(enabled));
		class_add_element("lighting",		TypeBool,		GetDAPicture3D(lighting));
		class_add_element("world_3d",		TypeBool,		GetDAPicture3D(world_3d));
		class_add_element("z",				TypeFloat,		GetDAPicture3D(pos.z));
		class_add_element("matrix",			TypeMatrix,		GetDAPicture3D(_matrix));
		class_add_element("model",			TypeModelP,		GetDAPicture3D(model));
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Picture3d::__init_ext__)));
			func_add_param("m",			TypeModelP);
			func_add_param("mat",		TypeMatrix);
			func_add_param("z",			TypeFloat);
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Picture3d::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf((tmf)&Picture3d::__delete__)));
		class_add_func("__Draw", TypeVoid, x_p(mf((tmf)&Picture3d::Draw)));
		class_add_func("__Update", TypeVoid, x_p(mf((tmf)&Picture3d::Update)));
		class_add_func("OnIterate", TypeVoid, x_p(mf((tmf)&Picture3d::OnIterate)));
		class_add_func("OnHover", TypeVoid, x_p(mf((tmf)&Picture3d::OnHover)));
		class_add_func("OnClick", TypeVoid, x_p(mf((tmf)&Picture3d::OnClick)));
		class_add_func("OnMouseEnter", TypeVoid, x_p(mf((tmf)&Picture3d::OnMouseEnter)));
		class_add_func("OnMouseLeave", TypeVoid, x_p(mf((tmf)&Picture3d::OnMouseLeave)));
		class_add_func("IsMouseOver", TypeBool, x_p(mf((tmf)&Picture3d::IsMouseOver)));
		TypePicture3D->LinkVirtualTable();
	
	add_class(TypeLayer);
		class_set_vtable_x(Layer);
		TypeLayer->vtable = new VirtualTable[20];
		class_add_element("enabled",		TypeBool,		GetDALayer(enabled));
		class_add_element("pos",			TypeVector,		GetDALayer(pos));
		class_add_element("color",			TypeColor,		GetDALayer(_color));
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Layer::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("set_cur",	TypeBool);
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Layer::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf((tmf)&Layer::__delete__)));
		class_add_func("__Draw", TypeVoid, x_p(mf((tmf)&Layer::Draw)));
		class_add_func("__Update", TypeVoid, x_p(mf((tmf)&Layer::Update)));
		class_add_func("OnIterate", TypeVoid, x_p(mf((tmf)&Layer::OnIterate)));
		class_add_func("OnHover", TypeVoid, x_p(mf((tmf)&Layer::OnHover)));
		class_add_func("OnClick", TypeVoid, x_p(mf((tmf)&Layer::OnClick)));
		class_add_func("OnMouseEnter", TypeVoid, x_p(mf((tmf)&Layer::OnMouseEnter)));
		class_add_func("OnMouseLeave", TypeVoid, x_p(mf((tmf)&Layer::OnMouseLeave)));
		class_add_func("IsMouseOver", TypeBool, x_p(mf((tmf)&Layer::IsMouseOver)));
		class_add_func("add", TypeVoid, x_p(mf((tmf)&Layer::add)));
			func_add_param("p", TypeLayerP);
		TypeLayer->LinkVirtualTable();
	
	add_class(TypeText);
		class_set_vtable_x(Text);
		TypeText->vtable = new VirtualTable[20];
		class_add_element("enabled",		TypeBool,		GetDAText(enabled));
		class_add_element("centric",		TypeBool,		GetDAText(centric));
		class_add_element("vertical",		TypeBool,		GetDAText(vertical));
		class_add_element("font",			TypeInt,		GetDAText(font));
		class_add_element("pos",			TypeVector,		GetDAText(pos));
		class_add_element("size",			TypeFloat,		GetDAText(size));
		class_add_element("color",			TypeColor,		GetDAText(_color));
		class_add_element("text",			TypeString,		GetDAText(text));
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Text::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("size",		TypeFloat);
			func_add_param("c",			TypeColor);
			func_add_param("str",		TypeString);
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Text::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf((tmf)&Text::__delete__)));
		class_add_func("__Draw", TypeVoid, x_p(mf((tmf)&Text::Draw)));
		class_add_func("__Update", TypeVoid, x_p(mf((tmf)&Text::Update)));
		class_add_func("OnIterate", TypeVoid, x_p(mf((tmf)&Text::OnIterate)));
		class_add_func("OnHover", TypeVoid, x_p(mf((tmf)&Text::OnHover)));
		class_add_func("OnClick", TypeVoid, x_p(mf((tmf)&Text::OnClick)));
		class_add_func("OnMouseEnter", TypeVoid, x_p(mf((tmf)&Text::OnMouseEnter)));
		class_add_func("OnMouseLeave", TypeVoid, x_p(mf((tmf)&Text::OnMouseLeave)));
		class_add_func("IsMouseOver", TypeBool, x_p(mf((tmf)&Text::IsMouseOver)));
		TypeText->LinkVirtualTable();
	
	add_class(TypeParticle);
		class_set_vtable_x(Particle);
		TypeParticle->vtable = new VirtualTable[10];
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
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Particle::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("texture", TypeInt);
			func_add_param("ttl", TypeFloat);
			func_add_param("radius", TypeFloat);
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Particle::__init__)));
		class_add_func("setr", TypeVoid, x_p(mf((tmf)&Particle::setr)));
			func_add_param("pos", TypeVector);
			func_add_param("ang", TypeVector);
			func_add_param("texture", TypeInt);
			func_add_param("ttl", TypeFloat);
			func_add_param("radius", TypeFloat);
		class_add_func("__delete__", TypeVoid, x_p(mf((tmf)&Particle::__delete__)));
		class_add_func("OnIterate", TypeVoid, x_p(mf((tmf)&Particle::OnIterate)));
		TypeParticle->LinkVirtualTable();

	add_class(TypeBeam);
		class_set_vtable_x(Beam);
		TypeBeam->vtable = new VirtualTable[10];
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
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Beam::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("length", TypeVector);
			func_add_param("texture", TypeInt);
			func_add_param("ttl", TypeFloat);
			func_add_param("radius", TypeFloat);
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Beam::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf((tmf)&Beam::__delete__)));
		class_add_func("OnIterate", TypeVoid, x_p(mf((tmf)&Beam::OnIterate)));
		TypeBeam->LinkVirtualTable();
	
	add_class(TypeEffect);
		class_set_vtable_x(Effect);
		TypeEffect->vtable = new VirtualTable[10];
		class_add_element("enabled",		TypeBool,		GetDAEffect(enabled));
		class_add_element("suicidal",		TypeBool,		GetDAEffect(suicidal));
		class_add_element("pos",			TypeVector,		GetDAEffect(pos));
		class_add_element("vel",			TypeVector,		GetDAEffect(vel));
		class_add_element("time_to_live",	TypeFloat,		GetDAEffect(time_to_live));
		class_add_element("func_delta_t",	TypeFloat,		GetDAEffect(func_delta_t));
		class_add_element("elapsed",		TypeFloat,		GetDAEffect(elapsed));
		class_add_element("model",			TypeModelP,		GetDAEffect(model));
		class_add_element("vertex",			TypeInt,		GetDAEffect(vertex));
		class_add_func("__init__", TypeVoid, x_p(mf((tmf)&Effect::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf((tmf)&Effect::__delete__)));
		class_add_func("OnIterate", TypeVoid, x_p(mf((tmf)&Effect::OnIterate)));
		class_add_func("OnEnable", TypeVoid, x_p(mf((tmf)&Effect::OnEnable)));
			func_add_param("enabled", TypeBool);
		TypeEffect->LinkVirtualTable();

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

	add_class(TypeTraceData);
		class_add_element("type",		TypeInt,		GetDATraceData(type));
		class_add_element("point",		TypeVector,		GetDATraceData(point));
		class_add_element("terrain",	TypeTerrainP,	GetDATraceData(terrain));
		class_add_element("model",		TypeModelP,		GetDATraceData(model));
		class_add_element("object",		TypeModelP,		GetDATraceData(object));

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
		class_add_element("parent",			TypeModelP,		GetDAModel(parent));
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
		//class_add_element("var_p",			TypePointerList,GetDAModel(script_var));
		class_add_element("data",			TypePointer,	GetDAModel(script_data));
		class_add_element("item",			TypeModelPList,	GetDAModel(inventary));
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
		class_add_func("AddForce",		TypeVoid,	x_p(mf((tmf)&Object::AddForce)));
			func_add_param("force",		TypeVector);
			func_add_param("rho",		TypeVector);
		class_add_func("AddTorque",		TypeVoid,	x_p(mf((tmf)&Object::AddTorque)));
			func_add_param("torque",		TypeVector);
		class_add_func("MakeVisible",		TypeVoid,		x_p(mf((tmf)&Object::MakeVisible)));
			func_add_param("visible",		TypeBool);
		class_add_func("UpdateData",		TypeVoid,		x_p(mf((tmf)&Object::UpdateData)));
/*	add_func("CalcMove",					TypeVoid,		x_p(mf((tmf)&Model::CalcMove)));
	add_func("Draw",						TypeVoid,		x_p(mf((tmf)&Model::Draw)));
		func_add_param("skin",				TypeInt);
		func_add_param("fx",				TypeBool);*/
		class_add_func("GetVertex",		TypeVector,		amd64_wrap(mf((tmf)&Model::GetVertex), &amd64_model_get_vertex));
			func_add_param("index",			TypeInt);
			func_add_param("skin",			TypeInt);
		class_add_func("ResetAnimation",		TypeVoid,		x_p(mf((tmf)&Model::ResetAnimation)));
		class_add_func("Animate",				TypeBool,		x_p(mf((tmf)&Model::Animate)));
			func_add_param("operation",		TypeInt);
			func_add_param("param1",		TypeFloat);
			func_add_param("param2",		TypeFloat);
			func_add_param("move",			TypeInt);
			func_add_param("time",			TypeFloatPs);
			func_add_param("dt",			TypeFloat);
			func_add_param("v",				TypeFloat);
			func_add_param("loop",			TypeBool);
		class_add_func("GetFrames",		TypeInt,		x_p(mf((tmf)&Model::GetFrames)));
			func_add_param("move",			TypeInt);
		class_add_func("BeginEditAnimation",	TypeVoid,		x_p(mf((tmf)&Model::BeginEditAnimation)));
		class_add_func("MakeEditable",		TypeVoid,		x_p(mf((tmf)&Model::MakeEditable)));
		class_add_func("BeginEdit",		TypeVoid,		x_p(mf((tmf)&Model::BeginEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("EndEdit",			TypeVoid,		x_p(mf((tmf)&Model::EndEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("SetBoneModel",		TypeVoid,		x_p(mf((tmf)&Model::SetBoneModel)));
			func_add_param("index",			TypeInt);
			func_add_param("bone",			TypeModelP);
		class_add_func("GetFilename",		TypeString,		x_p(mf((tmf)&Model::GetFilename)));
		class_add_func("GetRoot",			TypeModelP,		x_p(mf((tmf)&Model::GetRoot)));

	add_class(TypeTerrain);
		class_add_element("pos",			TypeVector,		GetDATerrain(pos));
		class_add_element("num_x",			TypeInt,		GetDATerrain(num_x));
		class_add_element("num_z",			TypeInt,		GetDATerrain(num_z));
		class_add_element("height",			TypeFloatList,	GetDATerrain(height));
		class_add_element("pattern",		TypeVector,		GetDATerrain(pattern));
		class_add_element("material",		TypeMaterialP,	GetDATerrain(material));
		class_add_element("texture_scale",	TypeVectorArray,GetDATerrain(texture_scale));
		class_add_func("Update",			TypeVoid,		x_p(mf((tmf)&Terrain::Update)));
			func_add_param("x1",		TypeInt);
			func_add_param("x2",		TypeInt);
			func_add_param("z1",		TypeInt);
			func_add_param("z2",		TypeInt);
			func_add_param("mode",		TypeInt);
		class_add_func("GetHeight",			TypeFloat,		x_p(mf((tmf)&Terrain::gimme_height)));
			func_add_param("p",			TypeVector);

	add_class(TypeCamera);
		class_set_vtable_x(Camera);
		TypeCamera->vtable = new VirtualTable[10];
		class_add_element("enabled",		TypeBool,		GetDACamera(enabled));
		class_add_element("show",			TypeBool,		GetDACamera(show));
		class_add_element("texture_out",	TypeInt,		GetDACamera(output_texture));
		class_add_element("texture_in",		TypeInt,		GetDACamera(input_texture));
		class_add_element("shader",			TypeInt,		GetDACamera(shader));
		class_add_element("shaded_displays",TypeBool,		GetDACamera(shaded_displays));
		class_add_element("pos",			TypeVector,		GetDACamera(pos));
		class_add_element("ang",			TypeVector,		GetDACamera(ang));
		class_add_element("vel",			TypeVector,		GetDACamera(vel));
		class_add_element("rot",			TypeVector,		GetDACamera(rot));
		class_add_element("zoom",			TypeFloat,		GetDACamera(zoom));
		class_add_element("scale_x",		TypeFloat,		GetDACamera(scale_x));
		class_add_element("dest",			TypeRect,		GetDACamera(dest));
		class_add_element("z",				TypeFloat,		GetDACamera(z));
		class_add_element("min_depth",		TypeFloat,		GetDACamera(min_depth));
		class_add_element("max_depth",		TypeFloat,		GetDACamera(max_depth));
		class_add_element("clipping_plane",	TypePlaneList,	GetDACamera(clipping_plane));
		class_add_element("ignore",			TypeModelPList,	GetDACamera(ignore));
		class_add_func("__init__",		TypeVoid,	x_p(mf((tmf)&Camera::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("ang",		TypeVector);
			func_add_param("dest",		TypeRect);
		class_add_func("__init__",		TypeVoid,	x_p(mf((tmf)&Camera::__init__)));
		class_add_func("__delete__",		TypeVoid,	x_p(mf((tmf)&Camera::__delete__)));
		class_add_func("StartScript",		TypeVoid,	x_p(mf((tmf)&Camera::StartScript)));
			func_add_param("filename",		TypeString);
			func_add_param("dpos",			TypeVector);
		class_add_func("StopScript",		TypeVoid,	x_p(mf((tmf)&Camera::StopScript)));
		class_add_func("OnIterate",		TypeVoid,	x_p(mf((tmf)&Camera::OnIterate)));
		class_add_func("Project",		TypeVector,	amd64_wrap(mf((tmf)&Camera::Project), &amd64_camera_project));
			func_add_param("v",			TypeVector);
		class_add_func("Unproject",		TypeVector,	amd64_wrap(mf((tmf)&Camera::Unproject), &amd64_camera_unproject));
			func_add_param("v",			TypeVector);
		TypeCamera->LinkVirtualTable();
	
	add_class(TypeWorldData);
		class_add_element("filename",		TypeString,		GetDAWorld(filename));
		class_add_element("gravity",		TypeVector,		GetDAWorld(gravity));
		class_add_element("skybox",		TypeModelPList,		GetDAWorld(skybox));
		class_add_element("background",		TypeColor,		GetDAWorld(background));
		class_add_element("fog",		TypeFog,		GetDAWorld(fog));
		class_add_element("var",		TypeFloatList,		GetDAWorld(var));
		class_add_element("ambient",		TypeColor,		GetDAWorld(ambient));
		class_add_element("sun",		TypeInt,		GetDAWorld(sun));
		class_add_element("speed_of_sound",		TypeFloat,		GetDAWorld(speed_of_sound));

	add_class(TypeEngineData);
		class_add_element("app_name",		TypeString,		GetDAEngine(AppName));
		class_add_element("version",		TypeString,		GetDAEngine(Version));
		class_add_element("elapsed",		TypeFloat,		GetDAEngine(Elapsed));
		class_add_element("elapsed_rt",		TypeFloat,		GetDAEngine(ElapsedRT));
		class_add_element("time_scale",		TypeFloat,		GetDAEngine(TimeScale));
		class_add_element("fps_max",		TypeFloat,		GetDAEngine(FpsMax));
		class_add_element("fps_min",		TypeFloat,		GetDAEngine(FpsMin));
		class_add_element("initial_world",		TypeString,		GetDAEngine(InitialWorldFile));
		class_add_element("second_world",		TypeString,		GetDAEngine(SecondWorldFile));
		class_add_element("physics_enabled",		TypeBool,		GetDAEngine(PhysicsEnabled));
		class_add_element("collisions_enabled",		TypeBool,		GetDAEngine(CollisionsEnabled));
		class_add_element("debug",		TypeBool,		GetDAEngine(Debug));
		class_add_element("show_timings",		TypeBool,		GetDAEngine(ShowTimings));
		class_add_element("wire_mode",		TypeBool,		GetDAEngine(WireMode));
		class_add_element("console_enabled",		TypeBool,		GetDAEngine(ConsoleEnabled));
		class_add_element("record",		TypeBool,		GetDAEngine(Record));
		class_add_element("resetting_game",		TypeBool,		GetDAEngine(ResettingGame));
		class_add_element("shadow_light",		TypeInt,		GetDAEngine(ShadowLight));
		class_add_element("shadow_color",		TypeColor,		GetDAEngine(ShadowColor));
		class_add_element("shadow_lower_detail",		TypeBool,		GetDAEngine(ShadowLowerDetail));
		class_add_element("shadow_level",		TypeInt,		GetDAEngine(ShadowLevel));
		class_add_element("multisampling",		TypeInt,		GetDAEngine(Multisampling));
		class_add_element("detail_factor",		TypeFloat,		GetDAEngine(DetailLevel));
		//class_add_element("detail_factor_inv",		TypeFloat,		GetDAEngine(DetailFactorInv));
		class_add_element("default_font",		TypeInt,		GetDAEngine(DefaultFont));
		class_add_element("mirror_level_max",		TypeInt,		GetDAEngine(MirrorLevelMax));
	
	add_class(TypeNetworkData);
		class_add_element("enabled",		TypeBool,		GetDANetwork(Enabled));
		class_add_element("session_name",		TypeString,		GetDANetwork(SessionName));
		class_add_element("host_names",		TypeString,		GetDANetwork(HostNames));
		class_add_element("is_host",		TypeBool,		GetDANetwork(IAmHost));
		class_add_element("is_client",		TypeBool,		GetDANetwork(IAmClient));
		class_add_element("has_read",		TypeBool,		GetDANetwork(HasRead));
		class_add_element("has_written",		TypeBool,		GetDANetwork(HasWritten));
		class_add_element("sock_to_host",		TypeSocket,		GetDANetwork(SocketToHost));
		class_add_element("sock_to_client",		TypeSocketList,		GetDANetwork(SocketToClient));
		class_add_element("cur_sock",		TypeSocketP,		GetDANetwork(CurrentSocket));

	add_class(TypeHostData);
		class_add_element("host",		TypeString,		GetDAHostData(host));
		class_add_element("session",		TypeString,		GetDAHostData(session));
	
	add_class(TypeHostDataList);
		class_add_func("__init__",		TypeVoid,	x_p(mf((tmf)&HostDataList::__init__)));
		class_add_func("__delete__",		TypeVoid,	x_p(mf((tmf)&HostDataList::__delete__)));
		class_add_func("__assign__",		TypeVoid,	x_p(mf((tmf)&HostDataList::__assign__)));
			func_add_param("other",			TypeHostDataList);
	
	add_func("XFDrawStr",			TypeFloat,	x_p(&XFDrawStr));
		func_add_param("x",			TypeFloat);
		func_add_param("y",			TypeFloat);
		func_add_param("z",			TypeFloat);
		func_add_param("size",		TypeFloat);
		func_add_param("str",		TypeString);
		func_add_param("font",		TypeInt);
		func_add_param("centric",	TypeBool);
	add_func("XFDrawVertStr",		TypeFloat,	x_p(&XFDrawVertStr));
		func_add_param("x",			TypeFloat);
		func_add_param("y",			TypeFloat);
		func_add_param("z",			TypeFloat);
		func_add_param("size",		TypeFloat);
		func_add_param("str",		TypeString);
		func_add_param("font",		TypeInt);
	add_func("XFGetWidth",			TypeFloat,	x_p(&XFGetWidth));
		func_add_param("size",		TypeFloat);
		func_add_param("s",		TypeString);
		func_add_param("font",		TypeInt);
	add_func("LoadFont",			TypeInt,	x_p(&LoadFont));
		func_add_param("filename",		TypeString);
	add_func("LoadModel",												TypeModelP,	x_p(&LoadModel));
		func_add_param("filename",		TypeString);
/*	add_func("GetModelOID",												TypeInt,	x_p(&MetaGetModelOID));
		func_add_param("filename",		TypeString);*/
	
	// engine
	// effects
	add_func("LightCreate",							TypeInt,	x_p(&Light::Create));
	add_func("LightSetColors",			TypeVoid,	x_p(&Light::SetColors));
		func_add_param("index",		TypeInt);
		func_add_param("ambient",		TypeColor);
		func_add_param("diffuse",		TypeColor);
		func_add_param("specular",		TypeColor);
	add_func("LightSetDirectional",			TypeVoid,	x_p(&Light::SetDirectional));
		func_add_param("index",		TypeInt);
		func_add_param("dir",		TypeVector);
	add_func("LightSetRadial",					TypeVoid,	x_p(&Light::SetRadial));
		func_add_param("index",		TypeInt);
		func_add_param("pos",		TypeVector);
		func_add_param("radius",		TypeFloat);
	add_func("LightEnable",							TypeVoid,	x_p(&Light::Enable));
		func_add_param("index",		TypeInt);
		func_add_param("enabled",		TypeBool);
	add_func("LightDelete",							TypeVoid,	x_p(&Light::Delete));
		func_add_param("index",		TypeInt);
	// game
	add_func("ExitProgram",									TypeVoid,	x_p(ExitProgram));
	add_func("ScreenShot",									TypeVoid,	x_p(ScreenShot));
	add_func("FindHosts",									TypeHostDataList,	x_p(FindHosts));
	add_func("XDelete",											TypeVoid,	x_p(&MetaDelete));
		func_add_param("p",		TypePointer);
	add_func("XDeleteLater",						TypeVoid,	x_p(&MetaDeleteLater));
		func_add_param("p",		TypePointer);
	add_func("XDeleteSelection",						TypeVoid,	x_p(&MetaDeleteSelection));
	add_func("LoadWorld",									TypeVoid,	x_p(LoadWorldSoon));
		func_add_param("filename",		TypeString);
	add_func("LoadGameFromHost",					TypeVoid,	x_p(LoadGameFromHostSoon));
		func_add_param("host",		TypeHostData);
	add_func("SaveGameState",							TypeVoid,	x_p(SaveGameState));
		func_add_param("filename",		TypeString);
	add_func("LoadGameState",							TypeVoid,	x_p(LoadGameStateSoon));
		func_add_param("filename",		TypeString);
	add_func("GetObjectByName",							TypeModelP,	x_p(&GetObjectByName));
		func_add_param("name",		TypeString);
	add_func("FindObjects",								TypeInt,	x_p(&GodFindObjects));
		func_add_param("pos",		TypeVector);
		func_add_param("radius",	TypeFloat);
		func_add_param("mode",		TypeInt);
		func_add_param("o",			TypeModelPListPs);
	add_func("NextObject",									TypeBool,	x_p(&NextObject));
		func_add_param("o",		TypeModelPPs);
	add_func("CreateObject",							TypeModelP,	x_p(&_CreateObject));
		func_add_param("filename",		TypeString);
		func_add_param("pos",		TypeVector);
	add_func("SplashScreen",					TypeVoid,	x_p(DrawSplashScreen));
		func_add_param("status",		TypeString);
		func_add_param("progress",		TypeFloat);
	add_func("RenderScene",									TypeVoid, 	NULL);
	add_func("GetG",											TypeVector,	amd64_wrap(&GetG, &amd64_getg));
		func_add_param("pos",		TypeVector);
	add_func("Trace",											TypeBool,	x_p(&GodTrace));
		func_add_param("p1",		TypeVector);
		func_add_param("p2",		TypeVector);
		func_add_param("d",			TypeTraceData);
		func_add_param("simple_test",	TypeBool);
		func_add_param("o_ignore",		TypeInt);
	add_func("LinkAddSpring",							TypeInt,	x_p(&AddLinkSpring));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p1",		TypeVector);
		func_add_param("p2",		TypeVector);
		func_add_param("dx0",		TypeFloat);
		func_add_param("k",			TypeFloat);
	add_func("LinkAddBall",									TypeInt,	x_p(&AddLinkBall));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p",			TypeVector);
	add_func("LinkAddHinge",							TypeInt,	x_p(&AddLinkHinge));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p"	,		TypeVector);
		func_add_param("ax",		TypeVector);
	add_func("LinkAddHinge2",							TypeInt,	x_p(&AddLinkHinge2));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p"	,		TypeVector);
		func_add_param("ax1",		TypeVector);
		func_add_param("ax2",		TypeVector);
	add_func("LinkAddSlider",									TypeInt,	x_p(&AddLinkSlider));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("ax",		TypeVector);
	add_func("LinkAddUniversal",									TypeInt,	x_p(&AddLinkUniversal));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p",			TypeVector);
		func_add_param("ax1",		TypeVector);
		func_add_param("ax2",		TypeVector);
	add_func("LinkSetTorque",					TypeVoid,	x_p(&LinkSetTorque));
		func_add_param("link",		TypeInt);
		func_add_param("torque",	TypeFloat);
	add_func("LinkSetTorqueAxis",					TypeVoid,	x_p(&LinkSetTorqueAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);
		func_add_param("torque",	TypeFloat);
	add_func("LinkSetRange",					TypeVoid,	x_p(&LinkSetRange));
		func_add_param("link",		TypeInt);
		func_add_param("min",		TypeFloat);
		func_add_param("max",		TypeFloat);
	add_func("LinkSetRangeAxis",					TypeVoid,	x_p(&LinkSetRangeAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);
		func_add_param("min",		TypeFloat);
		func_add_param("max",		TypeFloat);
	add_func("LinkSetFriction",		TypeVoid,	x_p(&LinkSetFriction));
		func_add_param("link",		TypeInt);
		func_add_param("friction",	TypeFloat);
	/*add_func("LinkSetFrictionAxis",		TypeVoid,	x_p(&LinkSetFrictionAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);
		func_add_param("friction",	TypeFloat);*/
	add_func("LinkGetPosition",					TypeFloat,	x_p(&LinkGetPosition));
		func_add_param("link",		TypeInt);
	add_func("LinkGetPositionAxis",					TypeFloat,	x_p(&LinkGetPositionAxis));
		func_add_param("link",		TypeInt);
		func_add_param("axis",		TypeInt);

	

	// game variables
	add_ext_var("World", 			TypeWorldData,	x_p(&World));
	add_ext_var("Engine", 			TypeEngineData,	x_p(&Engine));
	add_ext_var("Net", 				TypeNetworkData,x_p(&Net));
	add_ext_var("Object",			TypeModelPList,	x_p(&Objects));
	add_ext_var("Ego",				TypeModelP,		x_p(&Ego));
	add_ext_var("Terrain",			TypeTerrainPList,x_p(&Terrains));
	add_ext_var("Cam",				TypeCameraP,		x_p(&Cam));
	add_ext_var("CurrentLayer",		TypeLayerP,	x_p(&CurrentGrouping));

	
	// model skins
	add_const("SkinHigh",   TypeInt, x_p(SkinHigh));
	add_const("SkinMedium", TypeInt, x_p(SkinMedium));
	add_const("SkinLow",    TypeInt, x_p(SkinLow));
	// trace
	add_const("TraceTypeNone",    TypeInt, x_p(TraceTypeNone));
	add_const("TraceTypeTerrain", TypeInt, x_p(TraceTypeTerrain));
	add_const("TraceTypeModel",   TypeInt, x_p(TraceTypeModel));
	// animation operations
	add_const("MoveOpSet",         TypeInt, x_p(MoveOpSet));
	add_const("MoveOpSetNewKeyed", TypeInt, x_p(MoveOpSetNewKeyed));
	add_const("MoveOpSetOldKeyed", TypeInt, x_p(MoveOpSetOldKeyed));
	add_const("MoveOpAdd1Factor",  TypeInt, x_p(MoveOpAdd1Factor));
	add_const("MoveOpMix1Factor",  TypeInt, x_p(MoveOpMix1Factor));
	add_const("MoveOpMix2Factor",  TypeInt, x_p(MoveOpMix2Factor));
	

#if _X_ALLOW_X_
	AllowXContainer = true;
#endif
}

};
