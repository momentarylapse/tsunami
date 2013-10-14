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
	void _cdecl ExitProgram();
	void _cdecl ScreenShot();
	void _cdecl LoadWorldSoon(const string &filename);
	void _cdecl LoadGameFromHostSoon(const HostData &host);
	void _cdecl SaveGameState(const string &filename);
	void _cdecl LoadGameStateSoon(const string &filename);
	void _cdecl DrawSplashScreen(const string &str, float per);
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
Type *TypePicture3D;
Type *TypeLayer;
Type *TypeLayerP;
Type *TypeParticle;
Type *TypeParticleRot;
Type *TypeBeam;
Type *TypeEffect;
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
Type *TypeLight;
Type *TypeLightP;
Type *TypeTraceData;
Type *TypeTerrain;
Type *TypeTerrainP;
Type *TypeTerrainPList;
Type *TypeLink;
Type *TypeLinkP;
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
	#define	GetDAFog(x)			long(&_fog->x)-long(_fog)
	static Light::Light *_light;
	#define	GetDALight(x)			long(&_light->x)-long(_light)
	static WorldData *_world_data;
	#define	GetDAWorld(x)			long(&_world_data->x)-long(_world_data)
	static EngineData *_engine_data;
	#define	GetDAEngine(x)			long(&_engine_data->x)-long(_engine_data)
	static NetworkData *_network_data;
	#define	GetDANetwork(x)			long(&_network_data->x)-long(_network_data)
	static HostData *_host_data;
	#define	GetDAHostData(x)			long(&_host_data->x)-long(_host_data)
	class HostDataList : public Array<HostData>
	{
	public:
		void _cdecl __assign__(HostDataList &other){*this = other;}
	};
	static Camera *_camera;
	#define	GetDACamera(x)		long(&_camera->x)-long(_camera)
	static Terrain *_terrain;
	#define	GetDATerrain(x)		long(&_terrain->x)-long(_terrain)
	static TraceData *_tracedata;
	#define	GetDATraceData(x)		long(&_tracedata->x)-long(_tracedata)
	#define class_set_vtable_x(x)	class_set_vtable(x)
	static Link *_link;
	#define	GetDALink(x)			long(&_link->x)-long(_link)
#else
	typedef int Picture;
	typedef int Picture3d;
	typedef int Text;
	typedef int Layer;
	typedef int Particle;
	typedef int ParticleRot;
	typedef int Effect;
	typedef int Camera;
	typedef int Fog;
	namespace Light{
	typedef int Light;
	};
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
	#define	GetDALight(x)		0
	#define	GetDAWorld(x)		0
	#define	GetDAEngine(x)		0
	#define	GetDANetwork(x)		0
	#define	GetDAHostData(x)	0
	#define	GetDACamera(x)		0
	#define	GetDATerrain(x)		0
	#define	GetDATraceData(x)	0
	#define	GetDALink(x)			0
	typedef int TraceData;
	typedef int Bone;
	typedef int Model;
	typedef int Terrain;
	typedef int Skin;
	typedef int SubSkin;
	typedef int HostData;
	typedef int Material;
	typedef int Link;
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

	
	TypeModel			= add_type  ("Model",		sizeof(Model));
	TypeModelP			= add_type_p("Model*",		TypeModel);
	TypeModelPPs		= add_type_p("Model*&",		TypeModelP);
	TypeModelPList		= add_type_a("Model*[]",	TypeModelP, -1);
	TypeModelPListPs	= add_type_p("Model*[]&",	TypeModelPList, FLAG_SILENT);
	TypeBone			= add_type  ("Bone",		sizeof(Bone));
	TypeBoneList		= add_type_a("Bone[]",		TypeBone, -1);
	TypeText			= add_type  ("Text",		sizeof(Text));
	TypeTextP			= add_type_p("Text*",		TypeText);
	TypePicture			= add_type  ("Picture",		sizeof(Picture));
	TypePicture3D		= add_type  ("Picture3d",	sizeof(Picture3d));
	TypeLayer			= add_type  ("Layer",		sizeof(Layer));
	TypeLayerP			= add_type_p("Layer*",		TypeLayer);
	TypeParticle		= add_type  ("Particle",	sizeof(Particle));
	TypeParticleRot		= add_type  ("ParticleRot",	sizeof(ParticleRot));
	TypeBeam			= add_type  ("Beam",		sizeof(Particle));
	TypeEffect			= add_type  ("Effect",		sizeof(Effect));
	TypeCamera			= add_type  ("Camera",		sizeof(Camera));
	TypeCameraP			= add_type_p("Camera*",		TypeCamera);
	TypeSkin			= add_type  ("Skin",		sizeof(Skin));
	TypeSkinP			= add_type_p("Skin*",		TypeSkin);
	TypeSkinPArray		= add_type_a("Skin*[?]",	TypeSkinP, 1);
	TypeSubSkin			= add_type  ("SubSkin",		sizeof(SubSkin));
	TypeSubSkinList		= add_type_a("SubSkin[]",	TypeSubSkin, -1);
	TypeMaterial		= add_type  ("Material",	sizeof(Material));
	TypeMaterialP		= add_type_p("Material*",	TypeMaterial);
	TypeMaterialList	= add_type_a("Material[]",	TypeMaterial, -1);
	TypeFog				= add_type  ("Fog",			sizeof(Fog));
	TypeLight			= add_type  ("Light",		sizeof(Light::Light));
	TypeLightP			= add_type_p("Light*",		TypeLight);
	TypeTraceData		= add_type  ("TraceData",	sizeof(TraceData));
	TypeTerrain			= add_type  ("Terrain",		sizeof(Terrain));
	TypeTerrainP		= add_type_p("Terrain*",	TypeTerrain);
	TypeTerrainPList	= add_type_a("Terrain*[]",	TypeTerrainP, -1);
	TypeLink			= add_type  ("Link",		sizeof(Link));
	TypeLinkP			= add_type_p("Link*",		TypeLink);
	TypeWorldData		= add_type  ("WorldData",	0);
	TypeEngineData		= add_type  ("EngineData",	0);
	TypeNetworkData		= add_type  ("NetworkData",	0);
	TypeHostData		= add_type  ("HostData",    sizeof(HostData));
	TypeHostDataList	= add_type_a("HostData[]",	TypeHostData, -1);
	

	// bone, subskin, material...

	add_class(TypeLayer);
		class_add_element("enabled",		TypeBool,		GetDALayer(enabled));
		class_add_element("pos",			TypeVector,		GetDALayer(pos));
		class_add_element("color",			TypeColor,		GetDALayer(_color));
		class_add_func("__init__", TypeVoid, x_p(mf(&Layer::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("set_cur",	TypeBool);
		class_add_func("__init__", TypeVoid, x_p(mf(&Layer::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Layer::__delete__)));
		class_add_func_virtual("__Draw", TypeVoid, x_p(mf(&Layer::Draw)));
		class_add_func_virtual("__Update", TypeVoid, x_p(mf(&Layer::Update)));
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Layer::OnIterate)));
		class_add_func_virtual("OnHover", TypeVoid, x_p(mf(&Layer::OnHover)));
		class_add_func_virtual("OnClick", TypeVoid, x_p(mf(&Layer::OnClick)));
		class_add_func_virtual("OnMouseEnter", TypeVoid, x_p(mf(&Layer::OnMouseEnter)));
		class_add_func_virtual("OnMouseLeave", TypeVoid, x_p(mf(&Layer::OnMouseLeave)));
		class_add_func_virtual("IsMouseOver", TypeBool, x_p(mf(&Layer::IsMouseOver)));
		class_add_func("add", TypeVoid, x_p(mf(&Layer::add)));
			func_add_param("p", TypeLayerP);
		class_set_vtable_x(Layer);

	add_class(TypePicture);
		TypePicture->DeriveFrom(TypeLayer, false);
	//	class_add_element("enabled",		TypeBool,		GetDAPicture(enabled));
	//	class_add_element("pos",			TypeVector,		GetDAPicture(pos));
	//	class_add_element("color",			TypeColor,		GetDAPicture(_color));
		class_add_element("tc_inverted",	TypeBool,		GetDAPicture(tc_inverted));
		class_add_element("width",			TypeFloat,		GetDAPicture(width));
		class_add_element("height",			TypeFloat,		GetDAPicture(height));
		class_add_element("texture",		TypeInt,		GetDAPicture(texture));
		class_add_element("source",			TypeRect,		GetDAPicture(source));
		class_add_element("shader",			TypeInt,		GetDAPicture(shader));
		class_add_func("__init__", TypeVoid, x_p(mf(&Picture::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("width",		TypeFloat);
			func_add_param("height",	TypeFloat);
			func_add_param("texture",	TypeInt);
		class_add_func("__init__", TypeVoid, x_p(mf(&Picture::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Picture::__delete__)));
		class_add_func_virtual("__Draw", TypeVoid, x_p(mf(&Picture::Draw)), true);
		class_add_func_virtual("__Update", TypeVoid, x_p(mf(&Picture::Update)), true);
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Picture::OnIterate)), true);
		class_add_func_virtual("OnHover", TypeVoid, x_p(mf(&Picture::OnHover)), true);
		class_add_func_virtual("OnClick", TypeVoid, x_p(mf(&Picture::OnClick)), true);
		class_add_func_virtual("OnMouseEnter", TypeVoid, x_p(mf(&Picture::OnMouseEnter)), true);
		class_add_func_virtual("OnMouseLeave", TypeVoid, x_p(mf(&Picture::OnMouseLeave)), true);
		class_add_func_virtual("IsMouseOver", TypeBool, x_p(mf(&Picture::IsMouseOver)), true);
		class_set_vtable_x(Picture);
	
	add_class(TypePicture3D);
		TypePicture3D->DeriveFrom(TypeLayer, false);
	//	class_add_element("enabled",		TypeBool,		GetDAPicture3D(enabled));
		class_add_element("lighting",		TypeBool,		GetDAPicture3D(lighting));
		class_add_element("world_3d",		TypeBool,		GetDAPicture3D(world_3d));
		class_add_element("z",				TypeFloat,		GetDAPicture3D(pos.z));
		class_add_element("matrix",			TypeMatrix,		GetDAPicture3D(_matrix));
		class_add_element("model",			TypeModelP,		GetDAPicture3D(model));
		class_add_func("__init__", TypeVoid, x_p(mf(&Picture3d::__init_ext__)));
			func_add_param("m",			TypeModelP);
			func_add_param("mat",		TypeMatrix);
			func_add_param("z",			TypeFloat);
		class_add_func("__init__", TypeVoid, x_p(mf(&Picture3d::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Picture3d::__delete__)));
		class_add_func_virtual("__Draw", TypeVoid, x_p(mf(&Picture3d::Draw)), true);
		class_add_func_virtual("__Update", TypeVoid, x_p(mf(&Picture3d::Update)), true);
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Picture3d::OnIterate)), true);
		class_add_func_virtual("OnHover", TypeVoid, x_p(mf(&Picture3d::OnHover)), true);
		class_add_func_virtual("OnClick", TypeVoid, x_p(mf(&Picture3d::OnClick)), true);
		class_add_func_virtual("OnMouseEnter", TypeVoid, x_p(mf(&Picture3d::OnMouseEnter)), true);
		class_add_func_virtual("OnMouseLeave", TypeVoid, x_p(mf(&Picture3d::OnMouseLeave)), true);
		class_add_func_virtual("IsMouseOver", TypeBool, x_p(mf(&Picture3d::IsMouseOver)), true);
		class_set_vtable_x(Picture3d);
	
	add_class(TypeText);
		TypeText->DeriveFrom(TypeLayer, false);
	//	class_add_element("enabled",		TypeBool,		GetDAText(enabled));
	//	class_add_element("pos",			TypeVector,		GetDAText(pos));
	//	class_add_element("color",			TypeColor,		GetDAText(_color));
		class_add_element("centric",		TypeBool,		GetDAText(centric));
		class_add_element("vertical",		TypeBool,		GetDAText(vertical));
		class_add_element("font",			TypeInt,		GetDAText(font));
		class_add_element("size",			TypeFloat,		GetDAText(size));
		class_add_element("text",			TypeString,		GetDAText(text));
		class_add_func("__init__", TypeVoid, x_p(mf(&Text::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("size",		TypeFloat);
			func_add_param("c",			TypeColor);
			func_add_param("str",		TypeString);
		class_add_func("__init__", TypeVoid, x_p(mf(&Text::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Text::__delete__)));
		class_add_func_virtual("__Draw", TypeVoid, x_p(mf(&Text::Draw)), true);
		class_add_func_virtual("__Update", TypeVoid, x_p(mf(&Text::Update)), true);
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Text::OnIterate)), true);
		class_add_func_virtual("OnHover", TypeVoid, x_p(mf(&Text::OnHover)), true);
		class_add_func_virtual("OnClick", TypeVoid, x_p(mf(&Text::OnClick)), true);
		class_add_func_virtual("OnMouseEnter", TypeVoid, x_p(mf(&Text::OnMouseEnter)), true);
		class_add_func_virtual("OnMouseLeave", TypeVoid, x_p(mf(&Text::OnMouseLeave)), true);
		class_add_func_virtual("IsMouseOver", TypeBool, x_p(mf(&Text::IsMouseOver)), true);
		class_set_vtable_x(Text);
	
	add_class(TypeParticle);
		class_add_element("enabled",		TypeBool,		GetDAParticle(enabled));
		class_add_element("suicidal",		TypeBool,		GetDAParticle(suicidal));
		class_add_element("pos",			TypeVector,		GetDAParticle(pos));
		class_add_element("vel",			TypeVector,		GetDAParticle(vel));
		class_add_element("time_to_live",	TypeFloat,		GetDAParticle(time_to_live));
		class_add_element("radius",			TypeFloat,		GetDAParticle(radius));
		class_add_element("color",			TypeColor,		GetDAParticle(_color));
		class_add_element("texture",		TypeInt,		GetDAParticle(texture));
		class_add_element("source",			TypeRect,		GetDAParticle(source));
		class_add_element("func_delta_t",	TypeFloat,		GetDAParticle(func_delta_t));
		class_add_element("elapsed",		TypeFloat,		GetDAParticle(elapsed));
		class_add_func("__init__", TypeVoid, x_p(mf(&Particle::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("texture", TypeInt);
			func_add_param("ttl", TypeFloat);
			func_add_param("radius", TypeFloat);
		class_add_func("__init__", TypeVoid, x_p(mf(&Particle::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Particle::__delete__)));
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Particle::OnIterate)));
		class_set_vtable_x(Particle);

	add_class(TypeParticleRot);
		TypeParticleRot->DeriveFrom(TypeParticle, false);
		class_add_element("ang", TypeVector, GetDAParticle(parameter));
		class_add_func("__init__", TypeVoid, x_p(mf(&ParticleRot::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("ang", TypeVector);
			func_add_param("texture", TypeInt);
			func_add_param("ttl", TypeFloat);
			func_add_param("radius", TypeFloat);
		class_add_func("__init__", TypeVoid, x_p(mf(&ParticleRot::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&ParticleRot::__delete__)));
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&ParticleRot::OnIterate)), true);
		class_set_vtable_x(ParticleRot);

	add_class(TypeBeam);
		TypeBeam->DeriveFrom(TypeParticle, false);
		class_add_element("length",			TypeVector,		GetDAParticle(parameter));
		class_add_func("__init__", TypeVoid, x_p(mf(&Beam::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("length", TypeVector);
			func_add_param("texture", TypeInt);
			func_add_param("ttl", TypeFloat);
			func_add_param("radius", TypeFloat);
		class_add_func("__init__", TypeVoid, x_p(mf(&Beam::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Beam::__delete__)));
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Beam::OnIterate)), true);
		class_set_vtable_x(Beam);
	
	add_class(TypeEffect);
		class_add_element("enabled",		TypeBool,		GetDAEffect(enabled));
		class_add_element("suicidal",		TypeBool,		GetDAEffect(suicidal));
		class_add_element("pos",			TypeVector,		GetDAEffect(pos));
		class_add_element("vel",			TypeVector,		GetDAEffect(vel));
		class_add_element("time_to_live",	TypeFloat,		GetDAEffect(time_to_live));
		class_add_element("func_delta_t",	TypeFloat,		GetDAEffect(func_delta_t));
		class_add_element("elapsed",		TypeFloat,		GetDAEffect(elapsed));
		class_add_element("model",			TypeModelP,		GetDAEffect(model));
		class_add_element("vertex",			TypeInt,		GetDAEffect(vertex));
		class_add_func("__init__", TypeVoid, x_p(mf(&Effect::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Effect::__delete__)));
		class_add_func_virtual("OnInit", TypeVoid, x_p(mf(&Effect::OnInit)));
		class_add_func_virtual("OnDelete", TypeVoid, x_p(mf(&Effect::OnDelete)));
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Effect::OnIterate)));
		class_add_func_virtual("OnEnable", TypeVoid, x_p(mf(&Effect::OnEnable)));
		class_set_vtable_x(Effect);


	add_class(TypeLight);
		class_add_element("enabled",		TypeBool,		GetDALight(enabled));
		class_add_element("directional",	TypeBool,		GetDALight(directional));
		class_add_element("pos",			TypeVector,		GetDALight(pos));
		class_add_element("dir",			TypeVector,		GetDALight(dir));
		class_add_func("__init__", TypeVoid, x_p(mf(&Light::Light::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf(&Light::Light::__delete__)));
		class_add_func("SetDirectional", TypeVoid, x_p(mf(&Light::Light::SetDirectional)));
			func_add_param("dir", TypeVector);
		class_add_func("SetRadial", TypeVoid, x_p(mf(&Light::Light::SetRadial)));
			func_add_param("pos", TypeVector);
			func_add_param("radius", TypeFloat);
		class_add_func("SetColors", TypeVoid, x_p(mf(&Light::Light::SetColors)));
			func_add_param("am", TypeColor);
			func_add_param("di", TypeColor);
			func_add_param("sp", TypeColor);

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

	add_class(TypeBoneList);
		class_add_func("__init__", TypeVoid, x_p(mf(&Array<Bone>::__init__)));

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
		class_add_func("__init__", TypeVoid, x_p(mf(&Model::__init__)));
		class_add_func_virtual("__delete__", TypeVoid, x_p(mf(&Model::__delete__)));
		class_add_func("AddForce",		TypeVoid,	x_p(mf(&Object::AddForce)));
			func_add_param("force",		TypeVector);
			func_add_param("rho",		TypeVector);
		class_add_func("AddTorque",		TypeVoid,	x_p(mf(&Object::AddTorque)));
			func_add_param("torque",		TypeVector);
		class_add_func("MakeVisible",		TypeVoid,		x_p(mf(&Object::MakeVisible)));
			func_add_param("visible",		TypeBool);
		class_add_func("UpdateData",		TypeVoid,		x_p(mf(&Object::UpdateData)));
/*	add_func("CalcMove",					TypeVoid,		x_p(mf(&Model::CalcMove)));
	add_func("Draw",						TypeVoid,		x_p(mf(&Model::Draw)));
		func_add_param("skin",				TypeInt);
		func_add_param("fx",				TypeBool);*/
		class_add_func("GetVertex",		TypeVector,		amd64_wrap(mf(&Model::GetVertex), &amd64_model_get_vertex));
			func_add_param("index",			TypeInt);
			func_add_param("skin",			TypeInt);
		class_add_func("ResetAnimation",		TypeVoid,		x_p(mf(&Model::ResetAnimation)));
		class_add_func("Animate",				TypeBool,		x_p(mf(&Model::Animate)));
			func_add_param("operation",		TypeInt);
			func_add_param("param1",		TypeFloat);
			func_add_param("param2",		TypeFloat);
			func_add_param("move",			TypeInt);
			func_add_param("time",			TypeFloatPs);
			func_add_param("dt",			TypeFloat);
			func_add_param("v",				TypeFloat);
			func_add_param("loop",			TypeBool);
		class_add_func("GetFrames",		TypeInt,		x_p(mf(&Model::GetFrames)));
			func_add_param("move",			TypeInt);
		class_add_func("BeginEditAnimation",	TypeVoid,		x_p(mf(&Model::BeginEditAnimation)));
		class_add_func("MakeEditable",		TypeVoid,		x_p(mf(&Model::MakeEditable)));
		class_add_func("BeginEdit",		TypeVoid,		x_p(mf(&Model::BeginEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("EndEdit",			TypeVoid,		x_p(mf(&Model::EndEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("SetBoneModel",		TypeVoid,		x_p(mf(&Model::SetBoneModel)));
			func_add_param("index",			TypeInt);
			func_add_param("bone",			TypeModelP);
		class_add_func("GetFilename",		TypeString,		x_p(mf(&Model::GetFilename)));
		class_add_func("GetRoot",			TypeModelP,		x_p(mf(&Model::GetRoot)));
		class_add_func_virtual("OnInit", TypeVoid, x_p(mf(&Model::OnInit)));
		class_add_func_virtual("OnDelete", TypeVoid, x_p(mf(&Model::OnDelete)));
		class_add_func_virtual("OnIterate", TypeVoid, x_p(mf(&Model::OnIterate)));
		class_add_func_virtual("OnCollide", TypeVoid, x_p(mf(&Model::OnCollideM)));
			func_add_param("m", TypeModelP);
		class_add_func_virtual("OnCollide", TypeVoid, x_p(mf(&Model::OnCollideT)));
			func_add_param("t", TypeTerrainP);
		class_set_vtable_x(Model);

	add_class(TypeModelPList);
		class_add_func("__init__", TypeVoid, x_p(mf(&Array<Model*>::__init__)));

	add_class(TypeTerrain);
		class_add_element("pos",			TypeVector,		GetDATerrain(pos));
		class_add_element("num_x",			TypeInt,		GetDATerrain(num_x));
		class_add_element("num_z",			TypeInt,		GetDATerrain(num_z));
		class_add_element("height",			TypeFloatList,	GetDATerrain(height));
		class_add_element("pattern",		TypeVector,		GetDATerrain(pattern));
		class_add_element("material",		TypeMaterialP,	GetDATerrain(material));
		class_add_element("texture_scale",	TypeVectorArray,GetDATerrain(texture_scale));
		class_add_func("Update",			TypeVoid,		x_p(mf(&Terrain::Update)));
			func_add_param("x1",		TypeInt);
			func_add_param("x2",		TypeInt);
			func_add_param("z1",		TypeInt);
			func_add_param("z2",		TypeInt);
			func_add_param("mode",		TypeInt);
		class_add_func("GetHeight",			TypeFloat,		x_p(mf(&Terrain::gimme_height)));
			func_add_param("p",			TypeVector);

	add_class(TypeCamera);
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
		class_add_func("__init__",		TypeVoid,	x_p(mf(&Camera::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("ang",		TypeVector);
			func_add_param("dest",		TypeRect);
		class_add_func("__init__",		TypeVoid,	x_p(mf(&Camera::__init__)));
		class_add_func_virtual("__delete__",		TypeVoid,	x_p(mf(&Camera::__delete__)));
		class_add_func("StartScript",		TypeVoid,	x_p(mf(&Camera::StartScript)));
			func_add_param("filename",		TypeString);
			func_add_param("dpos",			TypeVector);
		class_add_func("StopScript",		TypeVoid,	x_p(mf(&Camera::StopScript)));
		class_add_func_virtual("OnIterate",		TypeVoid,	x_p(mf(&Camera::OnIterate)));
		class_add_func("Project",		TypeVector,	amd64_wrap(mf(&Camera::Project), &amd64_camera_project));
			func_add_param("v",			TypeVector);
		class_add_func("Unproject",		TypeVector,	amd64_wrap(mf(&Camera::Unproject), &amd64_camera_unproject));
			func_add_param("v",			TypeVector);
		class_set_vtable_x(Camera);


	add_class(TypeLink);
		class_add_func("SetTorque", TypeVoid, x_p(mf(&Link::SetTorque)));
			func_add_param("torque", TypeFloat);
		class_add_func("SetTorqueAxis", TypeVoid, x_p(mf(&Link::SetTorqueAxis)));
			func_add_param("axis", TypeInt);
			func_add_param("torque", TypeFloat);
		class_add_func("SetRange", TypeVoid, x_p(mf(&Link::SetRange)));
			func_add_param("min", TypeFloat);
			func_add_param("max", TypeFloat);
		class_add_func("SetRangeAxis", TypeVoid, x_p(mf(&Link::SetRangeAxis)));
			func_add_param("axis", TypeInt);
			func_add_param("min", TypeFloat);
			func_add_param("max", TypeFloat);
		class_add_func("SetFriction", TypeVoid, x_p(mf(&Link::SetFriction)));
			func_add_param("friction", TypeFloat);
		/*class_add_func("SetFrictionAxis", TypeVoid, x_p(mf(&Link::SetFrictionAxis)));
			func_add_param("axis", TypeInt);
			func_add_param("friction", TypeFloat);*/
		class_add_func("GetPosition", TypeFloat, x_p(mf(&Link::GetPosition)));
		class_add_func("GetPositionAxis", TypeFloat, x_p(mf(&Link::GetPositionAxis)));
			func_add_param("axis", TypeInt);
	
	add_class(TypeWorldData);
		class_add_element("filename",		TypeString,		GetDAWorld(filename));
		class_add_element("gravity",		TypeVector,		GetDAWorld(gravity));
		class_add_element("skybox",		TypeModelPList,		GetDAWorld(skybox));
		class_add_element("background",		TypeColor,		GetDAWorld(background));
		class_add_element("fog",		TypeFog,		GetDAWorld(fog));
		class_add_element("var",		TypeFloatList,		GetDAWorld(var));
		class_add_element("ambient",		TypeColor,		GetDAWorld(ambient));
		class_add_element("sun",		TypeLightP,		GetDAWorld(sun));
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
		class_add_element("shadow_light",		TypeLightP,		GetDAEngine(ShadowLight));
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
		class_add_element("host", TypeString, GetDAHostData(host));
		class_add_element("session", TypeString, GetDAHostData(session));
		class_add_func("__init__", TypeVoid, x_p(mf(&HostData::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf(&HostData::__delete__)));
		class_add_func("__assign__", TypeVoid, x_p(mf(&HostData::operator=)));
			func_add_param("other", TypeHostData);
	
	add_class(TypeHostDataList);
		class_add_func("__init__", TypeVoid, x_p(mf(&HostDataList::__init__)));
		class_add_func("__delete__", TypeVoid, x_p(mf(&HostDataList::clear)));
		class_add_func("clear", TypeVoid, x_p(mf(&HostDataList::clear)));
		class_add_func("__assign__", TypeVoid, x_p(mf(&HostDataList::__assign__)));
			func_add_param("other", TypeHostDataList);
	
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
	// game
	add_func("ExitProgram",									TypeVoid,	x_p(ExitProgram));
	add_func("ScreenShot",									TypeVoid,	x_p(ScreenShot));
	add_func("FindHosts",									TypeHostDataList,	x_p(FindHosts));
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
	add_func("LinkAddSpring",							TypeLinkP,	x_p(&AddLinkSpring));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p1",		TypeVector);
		func_add_param("p2",		TypeVector);
		func_add_param("dx0",		TypeFloat);
		func_add_param("k",			TypeFloat);
	add_func("LinkAddBall",									TypeLinkP,	x_p(&AddLinkBall));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p",			TypeVector);
	add_func("LinkAddHinge",							TypeLinkP,	x_p(&AddLinkHinge));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p"	,		TypeVector);
		func_add_param("ax",		TypeVector);
	add_func("LinkAddHinge2",							TypeLinkP,	x_p(&AddLinkHinge2));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p"	,		TypeVector);
		func_add_param("ax1",		TypeVector);
		func_add_param("ax2",		TypeVector);
	add_func("LinkAddSlider",									TypeLinkP,	x_p(&AddLinkSlider));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("ax",		TypeVector);
	add_func("LinkAddUniversal",									TypeLinkP,	x_p(&AddLinkUniversal));
		func_add_param("o1",		TypeModelP);
		func_add_param("o2",		TypeModelP);
		func_add_param("p",			TypeVector);
		func_add_param("ax1",		TypeVector);
		func_add_param("ax2",		TypeVector);

	

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
