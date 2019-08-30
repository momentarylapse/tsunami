#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"
#include "exception.h"


#ifdef _X_ALLOW_X_
	#include "../../../world/world.h"
	#include "../../../world/object.h"
	#include "../../../world/model_manager.h"
	#include "../../../world/terrain.h"
	#include "../../../world/camera.h"
	#include "../../../world/controller.h"
	#include "../../../physics/links.h"
	#include "../../../gui/font.h"
	#include "../../../gui/gui.h"
	#include "../../../fx/fx.h"
	#include "../../../fx/light.h"
	#include "../../../meta.h"
	#include "../../../networking.h"
	#include "../../../input.h"

	void _cdecl ExitProgram();
	void _cdecl ScreenShot();
	void _cdecl LoadWorldSoon(const string &filename);
	void _cdecl LoadGameFromHostSoon(const HostData &host);
	void _cdecl SaveGameState(const string &filename);
	void _cdecl LoadGameStateSoon(const string &filename);
	void _cdecl DrawSplashScreen(const string &str, float per);
	using namespace Gui;
	using namespace Fx;
#endif



namespace Kaba{


#ifdef _X_ALLOW_X_
	#define x_p(p)		(void*)p

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


Gui::Font* __LoadFont(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return Gui::LoadFont(filename))
	return nullptr;
}

Model* __LoadModel(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return LoadModelFull(filename));
	return nullptr;
}

Model* __CreateObject(const string &filename, const vector &pos)
{
	KABA_EXCEPTION_WRAPPER(return GodCreateObject(filename, filename, pos, quaternion::ID));
	return nullptr;
}

#pragma GCC pop_options



#else
	#define x_p(p)		nullptr
#endif

extern int _class_override_num_params;


const Class *TypeBone;
const Class *TypeBoneList;
const Class *TypeModel;
const Class *TypeModelP;
const Class *TypeModelPPs;
const Class *TypeModelPList;
const Class *TypeModelPListPs;
const Class *TypeText;
const Class *TypeTextP;
const Class *TypePicture;
const Class *TypePicture3D;
const Class *TypeLayer;
const Class *TypeLayerP;
const Class *TypeFont;
const Class *TypeFontP;
const Class *TypeParticle;
const Class *TypeParticleRot;
const Class *TypeBeam;
const Class *TypeEffect;
const Class *TypeCamera;
const Class *TypeCameraP;
const Class *TypeController;
const Class *TypeSkin;
const Class *TypeSkinP;
const Class *TypeSkinPArray;
const Class *TypeSubSkin;
const Class *TypeSubSkinList;
const Class *TypeMaterial;
const Class *TypeMaterialP;
const Class *TypeMaterialPList;
const Class *TypeFog;
const Class *TypeLight;
const Class *TypeLightP;
const Class *TypeTraceData;
const Class *TypeTerrain;
const Class *TypeTerrainP;
const Class *TypeTerrainPList;
const Class *TypeLink;
const Class *TypeLinkSpring;
const Class *TypeLinkBall;
const Class *TypeLinkSlider;
const Class *TypeLinkHinge;
const Class *TypeLinkHinge2;
const Class *TypeLinkUniversal;
const Class *TypeEngineData;
const Class *TypeWorldData;
const Class *TypeNetworkData;
const Class *TypeHostData;
const Class *TypeHostDataList;

extern const Class *TypeMatrix;
extern const Class *TypeMatrix3;
extern const Class *TypeFloatList;
extern const Class *TypeIntList;
extern const Class *TypeVectorList;
extern const Class *TypeVectorArray;
extern const Class *TypeFloatArray;
extern const Class *TypeFloatPs;
extern const Class *TypePlaneList;
extern const Class *TypeSocketP;
extern const Class *TypeSocketPList;
//extern Type *TypeVertexBufferP; // -> script_data_nix.cpp
extern const Class *TypeTextureP; // -> script_data_nix.cpp
extern const Class *TypeTexturePList;
extern const Class *TypeShaderP;


#ifdef _X_ALLOW_X_
	static Text *_text;
	#define	GetDAText(x)		long(&_text->x)-long(_text)
	static Picture *_picture;
	#define	GetDAPicture(x)		long(&_picture->x)-long(_picture)
	static Picture3d *_picture3d;
	#define	GetDAPicture3D(x)	long(&_picture3d->x)-long(_picture3d)
	static Layer *_layer;
	#define	GetDALayer(x)	long(&_layer->x)-long(_layer)
	static Font *_font;
	#define	GetDAFont(x)		long(&_font->x)-long(_font)
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
	static Controller *_controller;
	#define	GetDAController(x)		long(&_controller->x)-long(_controller)
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
	typedef int Font;
	typedef int Particle;
	typedef int ParticleRot;
	typedef int Effect;
	typedef int Camera;
	typedef int Controller;
	typedef int Fog;
	namespace Light{
	typedef int Light;
	};
	#define	GetDAText(x)		0
	#define	GetDAPicture(x)		0
	#define	GetDAPicture3D(x)	0
	#define	GetDALayer(x)		0
	#define	GetDAFont(x)		0
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
	#define	GetDAController(x)	0
	#define	GetDATerrain(x)		0
	#define	GetDATraceData(x)	0
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
	TypeFont			= add_type  ("Font",		sizeof(Font));
	TypeFontP			= add_type_p("Font*",		TypeFont);
	TypeLayer			= add_type  ("Layer",		sizeof(Layer));
	TypeLayerP			= add_type_p("Layer*",		TypeLayer);
	TypeText			= add_type  ("Text",		sizeof(Text));
	TypeTextP			= add_type_p("Text*",		TypeText);
	TypePicture			= add_type  ("Picture",		sizeof(Picture));
	TypePicture3D		= add_type  ("Picture3d",	sizeof(Picture3d));
	TypeParticle		= add_type  ("Particle",	sizeof(Particle));
	TypeParticleRot		= add_type  ("ParticleRot",	sizeof(ParticleRot));
	TypeBeam			= add_type  ("Beam",		sizeof(Particle));
	TypeEffect			= add_type  ("Effect",		sizeof(Effect));
	TypeCamera			= add_type  ("Camera",		sizeof(Camera));
	TypeCameraP			= add_type_p("Camera*",		TypeCamera);
	TypeController		= add_type  ("Controller",	sizeof(Controller));
	TypeSkin			= add_type  ("Skin",		sizeof(Skin));
	TypeSkinP			= add_type_p("Skin*",		TypeSkin);
	TypeSkinPArray		= add_type_a("Skin*[?]",	TypeSkinP, 1);
	TypeSubSkin			= add_type  ("SubSkin",		sizeof(SubSkin));
	TypeSubSkinList		= add_type_a("SubSkin[]",	TypeSubSkin, -1);
	TypeMaterial		= add_type  ("Material",	sizeof(Material));
	TypeMaterialP		= add_type_p("Material*",	TypeMaterial);
	TypeMaterialPList	= add_type_a("Material*[]",	TypeMaterialP, -1);
	TypeFog				= add_type  ("Fog",			sizeof(Fog));
	TypeLight			= add_type  ("Light",		sizeof(Light::Light));
	TypeLightP			= add_type_p("Light*",		TypeLight);
	TypeTraceData		= add_type  ("TraceData",	sizeof(TraceData));
	TypeTerrain			= add_type  ("Terrain",		sizeof(Terrain));
	TypeTerrainP		= add_type_p("Terrain*",	TypeTerrain);
	TypeTerrainPList	= add_type_a("Terrain*[]",	TypeTerrainP, -1);
	TypeLink			= add_type  ("Link",		sizeof(Link));
	TypeLinkSpring		= add_type  ("LinkSpring",	sizeof(Link));
	TypeLinkBall		= add_type  ("LinkBall",	sizeof(Link));
	TypeLinkSlider		= add_type  ("LinkSlider",	sizeof(Link));
	TypeLinkHinge		= add_type  ("LinkHinge",	sizeof(Link));
	TypeLinkHinge2		= add_type  ("LinkHinge2",	sizeof(Link));
	TypeLinkUniversal	= add_type  ("LinkUniversal",	sizeof(Link));
	TypeWorldData		= add_type  ("WorldData",	0);
	TypeEngineData		= add_type  ("EngineData",	0);
	TypeNetworkData		= add_type  ("NetworkData",	0);
	TypeHostData		= add_type  ("HostData",    sizeof(HostData));
	TypeHostDataList	= add_type_a("HostData[]",	TypeHostData, -1);
	

	// bone, subskin, material...


	add_class(TypeFont);
		class_add_func("draw_str",			TypeFloat32,	x_p(mf(&Font::drawStr)));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("z",			TypeFloat32);
			func_add_param("size",		TypeFloat32);
			func_add_param("str",		TypeString);
			func_add_param("align",	TypeInt);
		class_add_func("get_width",			TypeFloat32,	x_p(mf(&Font::getWidth)));
			func_add_param("size",		TypeFloat32);
			func_add_param("s",		TypeString);

	add_class(TypeLayer);
		class_add_element("enabled",		TypeBool,		GetDALayer(enabled));
		class_add_element("pos",			TypeVector,		GetDALayer(pos));
		class_add_element("color",			TypeColor,		GetDALayer(_color));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Layer::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("set_cur",	TypeBool);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Layer::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Layer::__delete__)));
		class_add_func_virtual("__draw", TypeVoid, x_p(mf(&Layer::draw)));
		class_add_func_virtual("__update", TypeVoid, x_p(mf(&Layer::update)));
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Layer::on_iterate)));
			func_add_param("dt", TypeFloat32);
		class_add_func_virtual("on_hover", TypeVoid, x_p(mf(&Layer::on_hover)));
		class_add_func_virtual("on_click", TypeVoid, x_p(mf(&Layer::on_click)));
		class_add_func_virtual("on_enter", TypeVoid, x_p(mf(&Layer::on_enter)));
		class_add_func_virtual("on_leave", TypeVoid, x_p(mf(&Layer::on_leave)));
		class_add_func_virtual("is_mouse_over", TypeBool, x_p(mf(&Layer::is_mouse_over)));
		class_add_func("add", TypeVoid, x_p(mf(&Layer::add)));
			func_add_param("p", TypeLayerP);
		class_set_vtable_x(Layer);

	add_class(TypePicture);
		class_derive_from(TypeLayer, false, false);
	//	class_add_element("enabled",		TypeBool,		GetDAPicture(enabled));
	//	class_add_element("pos",			TypeVector,		GetDAPicture(pos));
	//	class_add_element("color",			TypeColor,		GetDAPicture(_color));
		class_add_element("tc_inverted",	TypeBool,		GetDAPicture(tc_inverted));
		class_add_element("width",			TypeFloat32,		GetDAPicture(width));
		class_add_element("height",			TypeFloat32,		GetDAPicture(height));
		class_add_element("texture",		TypeTextureP,		GetDAPicture(texture));
		class_add_element("source",			TypeRect,		GetDAPicture(source));
		class_add_element("shader",			TypeShaderP,	GetDAPicture(shader));
		_class_override_num_params = 999;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Picture::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("width",		TypeFloat32);
			func_add_param("height",	TypeFloat32);
			func_add_param("texture",	TypeTextureP);
		_class_override_num_params = 0;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Picture::__init__)), FLAG_OVERRIDE);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Picture::__delete__)), FLAG_OVERRIDE);
		class_add_func_virtual("__draw", TypeVoid, x_p(mf(&Picture::draw)), FLAG_OVERRIDE);
		class_add_func_virtual("__update", TypeVoid, x_p(mf(&Picture::update)), FLAG_OVERRIDE);
		_class_override_num_params = 1;
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Picture::on_iterate)), FLAG_OVERRIDE);
			func_add_param("dt", TypeFloat32);
		_class_override_num_params = 0;
		class_add_func_virtual("on_hover", TypeVoid, x_p(mf(&Picture::on_hover)), FLAG_OVERRIDE);
		class_add_func_virtual("on_click", TypeVoid, x_p(mf(&Picture::on_click)), FLAG_OVERRIDE);
		class_add_func_virtual("on_enter", TypeVoid, x_p(mf(&Picture::on_enter)), FLAG_OVERRIDE);
		class_add_func_virtual("on_leave", TypeVoid, x_p(mf(&Picture::on_leave)), FLAG_OVERRIDE);
		class_add_func_virtual("is_mouse_over", TypeBool, x_p(mf(&Picture::is_mouse_over)), FLAG_OVERRIDE);
		_class_override_num_params = -1;
		class_set_vtable_x(Picture);
	
	add_class(TypePicture3D);
		class_derive_from(TypeLayer, false, false);
	//	class_add_element("enabled",		TypeBool,		GetDAPicture3D(enabled));
		class_add_element("lighting",		TypeBool,		GetDAPicture3D(lighting));
		class_add_element("world_3d",		TypeBool,		GetDAPicture3D(world_3d));
		class_add_element("z",				TypeFloat32,		GetDAPicture3D(pos.z));
		class_add_element("matrix",			TypeMatrix,		GetDAPicture3D(_matrix));
		class_add_element("model",			TypeModelP,		GetDAPicture3D(model));
		_class_override_num_params = 999;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Picture3d::__init_ext__)));
			func_add_param("m",			TypeModelP);
			func_add_param("mat",		TypeMatrix);
			func_add_param("z",			TypeFloat32);
		_class_override_num_params = 0;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Picture3d::__init__)), FLAG_OVERRIDE);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Picture3d::__delete__)), FLAG_OVERRIDE);
		class_add_func_virtual("__draw", TypeVoid, x_p(mf(&Picture3d::draw)), FLAG_OVERRIDE);
		class_add_func_virtual("__update", TypeVoid, x_p(mf(&Picture3d::update)), FLAG_OVERRIDE);
		_class_override_num_params = 1;
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Picture3d::on_iterate)), FLAG_OVERRIDE);
			func_add_param("dt", TypeFloat32);
		_class_override_num_params = 0;
		class_add_func_virtual("on_hover", TypeVoid, x_p(mf(&Picture3d::on_hover)), FLAG_OVERRIDE);
		class_add_func_virtual("on_click", TypeVoid, x_p(mf(&Picture3d::on_click)), FLAG_OVERRIDE);
		class_add_func_virtual("on_enter", TypeVoid, x_p(mf(&Picture3d::on_enter)), FLAG_OVERRIDE);
		class_add_func_virtual("on_leave", TypeVoid, x_p(mf(&Picture3d::on_leave)), FLAG_OVERRIDE);
		class_add_func_virtual("is_mouse_over", TypeBool, x_p(mf(&Picture3d::is_mouse_over)), FLAG_OVERRIDE);
		_class_override_num_params = -1;
		class_set_vtable_x(Picture3d);
	
	add_class(TypeText);
		class_derive_from(TypeLayer, false, false);
	//	class_add_element("enabled",		TypeBool,		GetDAText(enabled));
	//	class_add_element("pos",			TypeVector,		GetDAText(pos));
	//	class_add_element("color",			TypeColor,		GetDAText(_color));
		class_add_element("align",		TypeInt,		GetDAText(align));
		class_add_element("font",			TypeFontP,		GetDAText(font));
		class_add_element("size",			TypeFloat32,		GetDAText(size));
		class_add_element("text",			TypeString,		GetDAText(text));
		_class_override_num_params = 999;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Text::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("size",		TypeFloat32);
			func_add_param("c",			TypeColor);
			func_add_param("str",		TypeString);
			_class_override_num_params = 0;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Text::__init__)), FLAG_OVERRIDE);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Text::__delete__)), FLAG_OVERRIDE);
		class_add_func_virtual("__draw", TypeVoid, x_p(mf(&Text::draw)), FLAG_OVERRIDE);
		class_add_func_virtual("__update", TypeVoid, x_p(mf(&Text::update)), FLAG_OVERRIDE);
		_class_override_num_params = 1;
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Text::on_iterate)), FLAG_OVERRIDE);
			func_add_param("dt", TypeFloat32);
		_class_override_num_params = 0;
		class_add_func_virtual("on_hover", TypeVoid, x_p(mf(&Text::on_hover)), FLAG_OVERRIDE);
		class_add_func_virtual("on_click", TypeVoid, x_p(mf(&Text::on_click)), FLAG_OVERRIDE);
		class_add_func_virtual("on_enter", TypeVoid, x_p(mf(&Text::on_enter)), FLAG_OVERRIDE);
		class_add_func_virtual("on_leave", TypeVoid, x_p(mf(&Text::on_leave)), FLAG_OVERRIDE);
		class_add_func_virtual("is_mouse_over", TypeBool, x_p(mf(&Text::is_mouse_over)), FLAG_OVERRIDE);
		class_add_func("getWidth", TypeFloat32, x_p(mf(&Text::width)));
		_class_override_num_params = -1;
		class_set_vtable_x(Text);
	
	add_class(TypeParticle);
		class_add_element("enabled",		TypeBool,		GetDAParticle(enabled));
		class_add_element("suicidal",		TypeBool,		GetDAParticle(suicidal));
		class_add_element("pos",			TypeVector,		GetDAParticle(pos));
		class_add_element("vel",			TypeVector,		GetDAParticle(vel));
		class_add_element("time_to_live",	TypeFloat32,		GetDAParticle(time_to_live));
		class_add_element("radius",			TypeFloat32,		GetDAParticle(radius));
		class_add_element("color",			TypeColor,		GetDAParticle(_color));
		class_add_element("texture",		TypeTextureP,	GetDAParticle(texture));
		class_add_element("source",			TypeRect,		GetDAParticle(source));
		class_add_element("func_delta_t",	TypeFloat32,		GetDAParticle(func_delta_t));
		class_add_element("elapsed",		TypeFloat32,		GetDAParticle(elapsed));
		_class_override_num_params = 999;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Particle::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("texture", TypeTextureP);
			func_add_param("ttl", TypeFloat32);
			func_add_param("radius", TypeFloat32);
		_class_override_num_params = 0;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Particle::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Particle::__delete__)));
		_class_override_num_params = 1;
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Particle::on_iterate)));
			func_add_param("dt", TypeFloat32);
		_class_override_num_params = -1;
		class_set_vtable_x(Particle);

	add_class(TypeParticleRot);
		class_derive_from(TypeParticle, false, false);
		class_add_element("ang", TypeVector, GetDAParticle(parameter));
		_class_override_num_params = 999;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&ParticleRot::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("ang", TypeVector);
			func_add_param("texture", TypeTextureP);
			func_add_param("ttl", TypeFloat32);
			func_add_param("radius", TypeFloat32);
		_class_override_num_params = 0;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&ParticleRot::__init__)), FLAG_OVERRIDE);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&ParticleRot::__delete__)), FLAG_OVERRIDE);
		_class_override_num_params = 1;
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&ParticleRot::on_iterate)), FLAG_OVERRIDE);
			func_add_param("dt", TypeFloat32);
		_class_override_num_params = -1;
		class_set_vtable_x(ParticleRot);

	add_class(TypeBeam);
		class_derive_from(TypeParticle, false, false);
		class_add_element("length",			TypeVector,		GetDAParticle(parameter));
		_class_override_num_params = 999;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Beam::__init_ext__)));
			func_add_param("pos", TypeVector);
			func_add_param("length", TypeVector);
			func_add_param("texture", TypeTextureP);
			func_add_param("ttl", TypeFloat32);
			func_add_param("radius", TypeFloat32);
		_class_override_num_params = 0;
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Beam::__init__)), FLAG_OVERRIDE);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Beam::__delete__)), FLAG_OVERRIDE);
		_class_override_num_params = 1;
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Beam::on_iterate)), FLAG_OVERRIDE);
			func_add_param("dt", TypeFloat32);
		_class_override_num_params = -1;
		class_set_vtable_x(Beam);
	
	add_class(TypeEffect);
		class_add_element("enabled",		TypeBool,		GetDAEffect(enabled));
		class_add_element("suicidal",		TypeBool,		GetDAEffect(suicidal));
		class_add_element("pos",			TypeVector,		GetDAEffect(pos));
		class_add_element("vel",			TypeVector,		GetDAEffect(vel));
		class_add_element("time_to_live",	TypeFloat32,		GetDAEffect(time_to_live));
		class_add_element("func_delta_t",	TypeFloat32,		GetDAEffect(func_delta_t));
		class_add_element("elapsed",		TypeFloat32,		GetDAEffect(elapsed));
		class_add_element("model",			TypeModelP,		GetDAEffect(model));
		class_add_element("vertex",			TypeInt,		GetDAEffect(vertex));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Effect::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Effect::__delete__)));
		class_add_func_virtual("on_init", TypeVoid, x_p(mf(&Effect::on_init)));
		class_add_func_virtual("on_delete", TypeVoid, x_p(mf(&Effect::on_delete)));
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Effect::on_iterate)));
			func_add_param("dt", TypeFloat32);
		class_add_func_virtual("on_enable", TypeVoid, x_p(mf(&Effect::on_enable)));
		class_set_vtable_x(Effect);


	add_class(TypeLight);
		class_add_element("enabled",		TypeBool,		GetDALight(enabled));
		class_add_element("directional",	TypeBool,		GetDALight(directional));
		class_add_element("pos",			TypeVector,		GetDALight(pos));
		class_add_element("dir",			TypeVector,		GetDALight(dir));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Light::Light::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Light::Light::__delete__)));
		class_add_func("set_directional", TypeVoid, x_p(mf(&Light::Light::SetDirectional)));
			func_add_param("dir", TypeVector);
		class_add_func("set_radial", TypeVoid, x_p(mf(&Light::Light::SetRadial)));
			func_add_param("pos", TypeVector);
			func_add_param("radius", TypeFloat32);
		class_add_func("set_colors", TypeVoid, x_p(mf(&Light::Light::SetColors)));
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
		
	add_class(TypeSubSkinList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<SubSkin>::__init__);

	add_class(TypeMaterial);
		class_add_element("textures",		TypeTexturePList,	GetDAMaterial(textures));
		class_add_element("shader",			TypeShaderP,	GetDAMaterial(shader));
		class_add_element("alpha_factor",	TypeFloat32,		GetDAMaterial(alpha.factor));
		class_add_element("ambient",		TypeColor,		GetDAMaterial(ambient));
		class_add_element("diffuse",		TypeColor,		GetDAMaterial(diffuse));
		class_add_element("specular",		TypeColor,		GetDAMaterial(specular));
		class_add_element("emission",		TypeColor,		GetDAMaterial(emission));
		class_add_element("shininess",		TypeFloat32,		GetDAMaterial(shininess));

	add_class(TypeFog);
		class_add_element("enabled",		TypeBool,		GetDAFog(enabled));
		class_add_element("mode",			TypeInt,		GetDAFog(mode));
		class_add_element("start",			TypeFloat32,		GetDAFog(start));
		class_add_element("end",			TypeFloat32,		GetDAFog(end));
		class_add_element("density",		TypeFloat32,		GetDAFog(density));
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
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<Bone>::__init__);

	add_class(TypeModel);
		class_add_element("pos",			TypeVector,		GetDAModel(pos));
		class_add_element("vel",			TypeVector,		GetDAModel(vel));
		class_add_element("vel_surf",		TypeVector,		GetDAModel(vel_surf));
		class_add_element("ang",			TypeQuaternion,	GetDAModel(ang));
		class_add_element("rot",			TypeVector,		GetDAModel(rot));
		class_add_element("name",			TypeString,		GetDAModel(script_data.name));
		class_add_element("description",	TypeString,		GetDAModel(script_data.description));
		class_add_element("on_ground",		TypeBool,		GetDAModel(on_ground));
		class_add_element("ground_id",		TypeInt,		GetDAModel(ground_id));
		class_add_element("ground_normal",	TypeVector,		GetDAModel(ground_normal));
		class_add_element("g_factor",		TypeFloat32,		GetDAModel(physics_data.g_factor));
		class_add_element("object_id",		TypeInt,		GetDAModel(object_id));
		class_add_element("parent",			TypeModelP,		GetDAModel(parent));
		//class_add_element("visible",		TypeBool,		GetDAModel(visible));
		class_add_element("active_physics",	TypeBool,		GetDAModel(physics_data.active));
		class_add_element("passive_physics",	TypeBool,		GetDAModel(physics_data.passive));
		class_add_element("mass",			TypeFloat32,		GetDAModel(physics_data.mass));
		class_add_element("theta_0",		TypeMatrix3,	GetDAModel(physics_data.theta_0));
		class_add_element("theta",			TypeMatrix3,	GetDAModel(physics_data.theta));
		class_add_element("matrix",			TypeMatrix,		GetDAModel(_matrix));
		class_add_element("radius",			TypeFloat32,		GetDAModel(prop.radius));
		class_add_element("detail_dist",	TypeFloatArray,	GetDAModel(prop.detail_dist));
		class_add_element("var",			TypeFloatList,	GetDAModel(script_data.var));
		class_add_element("var_i",			TypeIntList,	GetDAModel(script_data.var));
		class_add_element("item",			TypeModelPList,	GetDAModel(script_data.inventary));
		class_add_element("skin",			TypeSkinPArray,	GetDAModel(skin));
		class_add_element("skin0",			TypeSkinP,		GetDAModel(skin[0]));
		class_add_element("skin1",			TypeSkinP,		GetDAModel(skin[1]));
		class_add_element("skin2",			TypeSkinP,		GetDAModel(skin[2]));
		class_add_element("material",		TypeMaterialPList,	GetDAModel(material));
		class_add_element("bone",			TypeBoneList,	GetDAModel(bone));
		class_add_element("min",			TypeVector,		GetDAModel(prop.min));
		class_add_element("max",			TypeVector,		GetDAModel(prop.max));
		class_add_element("test_collisions",	TypeBool,		GetDAModel(physics_data.test_collisions));
		class_add_element("allow_shadow",	TypeBool,		GetDAModel(prop.allow_shadow));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&Model::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&Model::__delete__)));
		class_add_func("add_force",		TypeVoid,	x_p(mf(&Object::add_force)));
			func_add_param("force",		TypeVector);
			func_add_param("rho",		TypeVector);
		class_add_func("add_torque",		TypeVoid,	x_p(mf(&Object::add_torque)));
			func_add_param("torque",		TypeVector);
		class_add_func("set_visible",		TypeVoid,		x_p(mf(&Object::make_visible)));
			func_add_param("visible",		TypeBool);
		class_add_func("update_data",		TypeVoid,		x_p(mf(&Object::update_data)));
/*	add_func("CalcMove",					TypeVoid,		x_p(mf(&Model::CalcMove)));
	add_func("Draw",						TypeVoid,		x_p(mf(&Model::Draw)));
		func_add_param("skin",				TypeInt);
		func_add_param("fx",				TypeBool);*/
		class_add_func("get_vertex",		TypeVector,		x_p(mf(&Model::GetVertex)));
			func_add_param("index",			TypeInt);
			func_add_param("skin",			TypeInt);
		class_add_func("reset_animation",		TypeVoid,		x_p(mf(&Model::ResetAnimation)));
		class_add_func("animate",				TypeBool,		x_p(mf(&Model::Animate)));
			func_add_param("operation",		TypeInt);
			func_add_param("param1",		TypeFloat32);
			func_add_param("param2",		TypeFloat32);
			func_add_param("move",			TypeInt);
			func_add_param("time",			TypeFloatPs);
			func_add_param("dt",			TypeFloat32);
			func_add_param("v",				TypeFloat32);
			func_add_param("loop",			TypeBool);
		class_add_func("get_frames",		TypeInt,		x_p(mf(&Model::GetFrames)));
			func_add_param("move",			TypeInt);
		class_add_func("begin_edit_animation",	TypeVoid,		x_p(mf(&Model::BeginEditAnimation)));
		class_add_func("make_editable",		TypeVoid,		x_p(mf(&Model::make_editable)));
		class_add_func("begin_edit",		TypeVoid,		x_p(mf(&Model::BeginEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("end_edit",			TypeVoid,		x_p(mf(&Model::EndEdit)));
			func_add_param("skin",			TypeInt);
		class_add_func("set_bone_model",		TypeVoid,		x_p(mf(&Model::SetBoneModel)));
			func_add_param("index",			TypeInt);
			func_add_param("bone",			TypeModelP);
		class_add_func("filename",		TypeString,		x_p(mf(&Model::filename)));
		class_add_func("root",			TypeModelP,		x_p(mf(&Model::root)));
		class_add_func_virtual("on_init", TypeVoid, x_p(mf(&Model::on_init)));
		class_add_func_virtual("on_delete", TypeVoid, x_p(mf(&Model::on_delete)));
		class_add_func_virtual("on_iterate", TypeVoid, x_p(mf(&Model::on_iterate)));
			func_add_param("dt", TypeFloat32);
		class_add_func_virtual("on_collide", TypeVoid, x_p(mf(&Model::on_collide_m)));
			func_add_param("m", TypeModelP);
		class_add_func_virtual("on_collide", TypeVoid, x_p(mf(&Model::on_collide_t)));
			func_add_param("t", TypeTerrainP);
		class_set_vtable_x(Model);

	add_class(TypeModelPList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<Model*>::__init__);

	add_class(TypeTerrain);
		class_add_element("pos",			TypeVector,		GetDATerrain(pos));
		class_add_element("num_x",			TypeInt,		GetDATerrain(num_x));
		class_add_element("num_z",			TypeInt,		GetDATerrain(num_z));
		class_add_element("height",			TypeFloatList,	GetDATerrain(height));
		class_add_element("pattern",		TypeVector,		GetDATerrain(pattern));
		class_add_element("material",		TypeMaterialP,	GetDATerrain(material));
		class_add_element("texture_scale",	TypeVectorArray,GetDATerrain(texture_scale));
		class_add_func("update",			TypeVoid,		x_p(mf(&Terrain::Update)));
			func_add_param("x1",		TypeInt);
			func_add_param("x2",		TypeInt);
			func_add_param("z1",		TypeInt);
			func_add_param("z2",		TypeInt);
			func_add_param("mode",		TypeInt);
		class_add_func("get_height",			TypeFloat32,		x_p(mf(&Terrain::gimme_height)));
			func_add_param("p",			TypeVector);

	add_class(TypeCamera);
		class_add_element("enabled",		TypeBool,		GetDACamera(enabled));
		class_add_element("show",			TypeBool,		GetDACamera(show));
		class_add_element("output",			TypeTextureP,	GetDACamera(output));
		class_add_element("input",			TypeTextureP,	GetDACamera(input));
		class_add_element("shader",			TypeShaderP,	GetDACamera(shader));
		class_add_element("override_shader",			TypeShaderP,	GetDACamera(override_shader));
		class_add_element("shaded_displays",TypeBool,		GetDACamera(shaded_displays));
		class_add_element("pos",			TypeVector,		GetDACamera(pos));
		class_add_element("ang",			TypeQuaternion,	GetDACamera(ang));
		class_add_element("vel",			TypeVector,		GetDACamera(vel));
		class_add_element("rot",			TypeVector,		GetDACamera(rot));
		class_add_element("m_all",			TypeMatrix,		GetDACamera(m_all));
		class_add_element("zoom",			TypeFloat32,		GetDACamera(zoom));
		class_add_element("scale_x",		TypeFloat32,		GetDACamera(scale_x));
		class_add_element("dest",			TypeRect,		GetDACamera(dest));
		class_add_element("z",				TypeFloat32,		GetDACamera(z));
		class_add_element("min_depth",		TypeFloat32,		GetDACamera(min_depth));
		class_add_element("max_depth",		TypeFloat32,		GetDACamera(max_depth));
		class_add_element("clipping_plane",	TypePlaneList,	GetDACamera(clipping_plane));
		class_add_element("ignore",			TypeModelPList,	GetDACamera(ignore));
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,	x_p(mf(&Camera::__init_ext__)));
			func_add_param("pos",		TypeVector);
			func_add_param("ang",		TypeQuaternion);
			func_add_param("dest",		TypeRect);
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,	x_p(mf(&Camera::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,	x_p(mf(&Camera::__delete__)));
		class_add_func("start",		TypeVoid,	x_p(mf(&Camera::start_script)));
			func_add_param("filename",		TypeString);
			func_add_param("dpos",			TypeVector);
		class_add_func("stop",		TypeVoid,	x_p(mf(&Camera::stop_script)));
		class_add_func_virtual("on_iterate",		TypeVoid,	x_p(mf(&Camera::on_iterate)));
			func_add_param("dt", TypeFloat32);
		class_add_func("project",		TypeVector,	x_p(mf(&Camera::project)));
			func_add_param("v",			TypeVector);
		class_add_func("unproject",		TypeVector,	x_p(mf(&Camera::unproject)));
			func_add_param("v",			TypeVector);
		class_set_vtable_x(Camera);


	add_class(TypeController);
		class_add_element("enabled",		TypeBool,		GetDAController(enabled));
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,	x_p(mf(&Controller::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,	x_p(mf(&Controller::__delete__)));
		class_add_func_virtual("on_init",		TypeVoid,	x_p(mf(&Controller::on_init)));
		class_add_func_virtual("on_delete",		TypeVoid,	x_p(mf(&Controller::on_delete)));
		class_add_func_virtual("on_iterate",		TypeVoid,	x_p(mf(&Controller::on_iterate)));
			func_add_param("dt", TypeFloat32);
		class_add_func_virtual("on_input",		TypeVoid,	x_p(mf(&Controller::on_input)));
		class_add_func_virtual("on_draw_pre",		TypeVoid,	x_p(mf(&Controller::on_draw_pre)));
		class_add_func_virtual("on_draw_post",		TypeVoid,	x_p(mf(&Controller::on_draw_post)));
		class_add_func_virtual("on_input_pre",		TypeVoid,	x_p(mf(&Controller::on_input_pre)));
		class_add_func_virtual("on_input_post",		TypeVoid,	x_p(mf(&Controller::on_input_post)));
		class_add_func_virtual("on_iterate_pre",		TypeVoid,	x_p(mf(&Controller::on_iterate_pre)));
		class_add_func_virtual("on_iterate_post",		TypeVoid,	x_p(mf(&Controller::on_iterate_post)));
		class_add_func_virtual("on_mouse_wheel",		TypeVoid,	x_p(mf(&Controller::on_mouse_wheel)));
			func_add_param("dz", TypeFloat32);
		class_add_func_virtual("on_left_button_down",		TypeVoid,	x_p(mf(&Controller::on_left_button_down)));
		class_add_func_virtual("on_left_button_up",		TypeVoid,	x_p(mf(&Controller::on_left_button_up)));
		class_add_func_virtual("on_right_button_down",		TypeVoid,	x_p(mf(&Controller::on_right_button_down)));
		class_add_func_virtual("on_right_button_up",		TypeVoid,	x_p(mf(&Controller::on_right_button_up)));
		class_add_func_virtual("on_middle_button_down",		TypeVoid,	x_p(mf(&Controller::on_middle_button_down)));
		class_add_func_virtual("on_middle_button_up",		TypeVoid,	x_p(mf(&Controller::on_middle_button_up)));
		class_add_func_virtual("on_key_down",		TypeVoid,	x_p(mf(&Controller::on_key_down)));
			func_add_param("key", TypeInt);
		class_add_func_virtual("onKey_up",		TypeVoid,	x_p(mf(&Controller::on_key_up)));
			func_add_param("key", TypeInt);
		class_add_func_virtual("on_add_client",		TypeVoid,	x_p(mf(&Controller::on_add_client)));
		class_add_func_virtual("on_remove_client",		TypeVoid,	x_p(mf(&Controller::on_remove_client)));
		class_add_func("add_net_msg_handler",		TypeVoid,			x_p(mf(&Controller::add_net_msg_handler)));
			func_add_param("name",		TypeString);
			func_add_param("func",		TypeFunctionCodeP);
		class_add_func("start_net_msg",		TypeVoid,			x_p(mf(&Controller::start_net_msg)));
			func_add_param("name",		TypeString);
			func_add_param("target",		TypeInt);
		class_add_func("end_net_msg",		TypeVoid,			x_p(mf(&Controller::end_net_msg)));
		class_set_vtable_x(Controller);


	add_class(TypeLink);
		class_add_func("set_torque", TypeVoid, x_p(mf(&Link::SetTorque)));
			func_add_param("torque", TypeFloat32);
		class_add_func("set_torque_axis", TypeVoid, x_p(mf(&Link::SetTorqueAxis)));
			func_add_param("axis", TypeInt);
			func_add_param("torque", TypeFloat32);
		class_add_func("set_range", TypeVoid, x_p(mf(&Link::SetRange)));
			func_add_param("min", TypeFloat32);
			func_add_param("max", TypeFloat32);
		class_add_func("set_range_axis", TypeVoid, x_p(mf(&Link::SetRangeAxis)));
			func_add_param("axis", TypeInt);
			func_add_param("min", TypeFloat32);
			func_add_param("max", TypeFloat32);
		class_add_func("set_friction", TypeVoid, x_p(mf(&Link::SetFriction)));
			func_add_param("friction", TypeFloat32);
		/*class_add_func("setFrictionAxis", TypeVoid, x_p(mf(&Link::SetFrictionAxis)));
			func_add_param("axis", TypeInt);
			func_add_param("friction", TypeFloat);*/
		class_add_func("get_position", TypeFloat32, x_p(mf(&Link::GetPosition)));
		class_add_func("get_position_axis", TypeFloat32, x_p(mf(&Link::GetPositionAxis)));
			func_add_param("axis", TypeInt);

	add_class(TypeLinkSpring);
		class_derive_from(TypeLink, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT,							TypeVoid,	x_p(mf(&Link::__init_spring__)));
			func_add_param("o1",		TypeModelP);
			func_add_param("o2",		TypeModelP);
			func_add_param("p1",		TypeVector);
			func_add_param("p2",		TypeVector);
			func_add_param("dx0",		TypeFloat32);
			func_add_param("k",			TypeFloat32);

	add_class(TypeLinkBall);
		class_derive_from(TypeLink, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT,							TypeVoid,	x_p(mf(&Link::__init_ball__)));
			func_add_param("o1",		TypeModelP);
			func_add_param("o2",		TypeModelP);
			func_add_param("p",			TypeVector);

	add_class(TypeLinkHinge);
		class_derive_from(TypeLink, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT,							TypeVoid,	x_p(mf(&Link::__init_hinge__)));
			func_add_param("o1",		TypeModelP);
			func_add_param("o2",		TypeModelP);
			func_add_param("p"	,		TypeVector);
			func_add_param("ax",		TypeVector);

	add_class(TypeLinkHinge2);
		class_derive_from(TypeLink, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT,							TypeVoid,	x_p(mf(&Link::__init_hinge2__)));
			func_add_param("o1",		TypeModelP);
			func_add_param("o2",		TypeModelP);
			func_add_param("p"	,		TypeVector);
			func_add_param("ax1",		TypeVector);
			func_add_param("ax2",		TypeVector);

	add_class(TypeLinkSlider);
		class_derive_from(TypeLink, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT,							TypeVoid,	x_p(mf(&Link::__init_slider__)));
			func_add_param("o1",		TypeModelP);
			func_add_param("o2",		TypeModelP);
			func_add_param("ax",		TypeVector);


	add_class(TypeLinkUniversal);
		class_derive_from(TypeLink, false, false);
		class_add_func(IDENTIFIER_FUNC_INIT,							TypeVoid,	x_p(mf(&Link::__init_universal__)));
			func_add_param("o1",		TypeModelP);
			func_add_param("o2",		TypeModelP);
			func_add_param("p",			TypeVector);
			func_add_param("ax1",		TypeVector);
			func_add_param("ax2",		TypeVector);
	
	add_class(TypeWorldData);
		class_add_element("filename",		TypeString,		GetDAWorld(filename));
		class_add_element("gravity",		TypeVector,		GetDAWorld(gravity));
		class_add_element("objects",		TypeModelPList,		GetDAWorld(objects));
		class_add_element("terrains",		TypeTerrainPList,		GetDAWorld(terrains));
		//class_add_element("ego",			TypeModelP,		GetDAWorld(ego));
		class_add_element("skybox",		TypeModelPList,		GetDAWorld(skybox));
		class_add_element("background",		TypeColor,		GetDAWorld(background));
		class_add_element("fog",		TypeFog,		GetDAWorld(fog));
		class_add_element("ambient",		TypeColor,		GetDAWorld(ambient));
		class_add_element("sun",		TypeLightP,		GetDAWorld(sun));
		class_add_element("speed_of_sound",		TypeFloat32,		GetDAWorld(speed_of_sound));

	add_class(TypeEngineData);
		class_add_element("app_name",		TypeString,		GetDAEngine(AppName));
		class_add_element("version",		TypeString,		GetDAEngine(Version));
		class_add_element("elapsed",		TypeFloat32,		GetDAEngine(Elapsed));
		class_add_element("elapsed_rt",		TypeFloat32,		GetDAEngine(ElapsedRT));
		class_add_element("time_scale",		TypeFloat32,		GetDAEngine(TimeScale));
		class_add_element("fps_max",		TypeFloat32,		GetDAEngine(FpsMax));
		class_add_element("fps_min",		TypeFloat32,		GetDAEngine(FpsMin));
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
		class_add_element("detail_factor",		TypeFloat32,		GetDAEngine(DetailLevel));
		//class_add_element("detail_factor_inv",		TypeFloat,		GetDAEngine(DetailFactorInv));
		class_add_element("default_font",		TypeFontP,		GetDAEngine(DefaultFont));
		class_add_element("mirror_level_max",		TypeInt,		GetDAEngine(MirrorLevelMax));
	
	add_class(TypeNetworkData);
		class_add_element("enabled",		TypeBool,		GetDANetwork(Enabled));
		class_add_element("session_name",		TypeString,		GetDANetwork(SessionName));
		class_add_element("host_names",		TypeString,		GetDANetwork(HostNames));
		class_add_element("is_host",		TypeBool,		GetDANetwork(IAmHost));
		class_add_element("is_client",		TypeBool,		GetDANetwork(IAmClient));
		class_add_element("has_read",		TypeBool,		GetDANetwork(HasRead));
		class_add_element("has_written",		TypeBool,		GetDANetwork(HasWritten));
		class_add_element("sock_to_host",		TypeSocketP,		GetDANetwork(SocketToHost));
		class_add_element("sock_to_client",		TypeSocketPList,		GetDANetwork(SocketToClient));
		class_add_element("cur_sock",		TypeSocketP,		GetDANetwork(CurrentSocket));

	add_class(TypeHostData);
		class_add_element("host", TypeString, GetDAHostData(host));
		class_add_element("session", TypeString, GetDAHostData(session));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&HostData::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&HostData::__delete__)));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, x_p(mf(&HostData::__assign__)));
			func_add_param("other", TypeHostData);
	
	add_class(TypeHostDataList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, x_p(mf(&HostDataList::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, x_p(mf(&HostDataList::clear)));
		class_add_func("clear", TypeVoid, x_p(mf(&HostDataList::clear)));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, x_p(mf(&HostDataList::__assign__)));
			func_add_param("other", TypeHostDataList);

	add_func("LoadFont",			TypeFontP,	x_p(&__LoadFont), FLAG_STATIC);
		func_add_param("filename",		TypeString);
	add_func("LoadModel",												TypeModelP,	x_p(&__LoadModel), FLAG_STATIC);
		func_add_param("filename",		TypeString);
/*	add_func("GetModelOID",												TypeInt,	x_p(&MetaGetModelOID), FLAG_STATIC);
		func_add_param("filename",		TypeString);*/
	
	// engine
	// game
	add_func("ExitProgram",									TypeVoid,	x_p(ExitProgram), FLAG_STATIC);
	add_func("ScreenShot",									TypeVoid,	x_p(ScreenShot), FLAG_STATIC);
	add_func("FindHosts",									TypeHostDataList,	x_p(FindHosts), FLAG_STATIC);
	add_func("LoadWorld",									TypeVoid,	x_p(LoadWorldSoon), FLAG_STATIC);
		func_add_param("filename",		TypeString);
	add_func("LoadGameFromHost",					TypeVoid,	x_p(LoadGameFromHostSoon), FLAG_STATIC);
		func_add_param("host",		TypeHostData);
	add_func("SaveGameState",							TypeVoid,	x_p(SaveGameState), FLAG_STATIC);
		func_add_param("filename",		TypeString);
	add_func("LoadGameState",							TypeVoid,	x_p(LoadGameStateSoon), FLAG_STATIC);
		func_add_param("filename",		TypeString);
	add_func("GetObjectByName",							TypeModelP,	x_p(&GetObjectByName), FLAG_STATIC);
		func_add_param("name",		TypeString);
	add_func("FindObjects",								TypeInt,	x_p(&GodFindObjects), FLAG_STATIC);
		func_add_param("pos",		TypeVector);
		func_add_param("radius",	TypeFloat32);
		func_add_param("mode",		TypeInt);
		func_add_param("o",			TypeModelPListPs);
	add_func("NextObject",									TypeBool,	x_p(&NextObject), FLAG_STATIC);
		func_add_param("o",		TypeModelPPs);
	add_func("CreateObject",							TypeModelP,	x_p(&__CreateObject), FLAG_STATIC);
		func_add_param("filename",		TypeString);
		func_add_param("pos",		TypeVector);
	add_func("SplashScreen",					TypeVoid,	x_p(DrawSplashScreen), FLAG_STATIC);
		func_add_param("status",		TypeString);
		func_add_param("progress",		TypeFloat32);
	add_func("RenderScene",									TypeVoid, 	nullptr, FLAG_STATIC);
	add_func("GetG",											TypeVector,	x_p(&GetG), FLAG_STATIC);
		func_add_param("pos",		TypeVector);
	add_func("Trace",											TypeBool,	x_p(&GodTrace), FLAG_STATIC);
		func_add_param("p1",		TypeVector);
		func_add_param("p2",		TypeVector);
		func_add_param("d",			TypeTraceData);
		func_add_param("simple_test",	TypeBool);
		func_add_param("o_ignore",		TypeModelP);
	

		// user input
	add_func("UpdateInput",			TypeVoid,	x_p(&update_input), FLAG_STATIC);
	add_func("GetKey",								TypeBool,	x_p(&get_key), FLAG_STATIC);
		func_add_param("id",		TypeInt);
	add_func("GetKeyDown",								TypeBool,	x_p(&get_key_down), FLAG_STATIC);
		func_add_param("id",		TypeInt);
	add_func("GetKeyDownRep",							TypeInt,	x_p(&get_key_down_rep), FLAG_STATIC);
	add_func("GetKeyUp",								TypeBool,	x_p(&get_key_up), FLAG_STATIC);
		func_add_param("id",		TypeInt);
	add_func("GetKeyChar",							TypeString,	x_p(&get_key_char), FLAG_STATIC);
		func_add_param("id",		TypeInt);
	add_func("GetButton",										TypeBool,	x_p(&get_button), FLAG_STATIC);
		func_add_param("button",	TypeInt);
	add_func("GetButtonDown",										TypeBool,	x_p(&get_button_down), FLAG_STATIC);
		func_add_param("button",	TypeInt);
	add_func("GetButtonUp",										TypeBool,	x_p(&get_button_up), FLAG_STATIC);
		func_add_param("button",	TypeInt);

	// game variables
	add_ext_var("world", 			TypeWorldData,	x_p(&World));
	add_ext_var("ego", 				TypeModelP,		x_p(&World.ego));
	add_ext_var("engine", 			TypeEngineData,	x_p(&Engine));
	add_ext_var("net", 				TypeNetworkData,x_p(&Net));
	add_ext_var("cam",				TypeCameraP,		x_p(&cam));
	add_ext_var("current_layer",		TypeLayerP,	x_p(&CurrentGrouping));

	add_ext_var("mouse_pixel",			TypeVector,		x_p(&mouse));
	add_ext_var("mouse",			TypeVector,		x_p(&mouse_rel));
	add_ext_var("dmouse_pixel",			TypeVector,		x_p(&mouse_d));
	add_ext_var("dmouse",		TypeVector,		x_p(&mouse_d_rel));

	// trace
	add_const("TRACE_NONE",    TypeInt, x_p(TRACE_TYPE_NONE));
	add_const("TRACE_TERRAIN", TypeInt, x_p(TRACE_TYPE_TERRAIN));
	add_const("TRACE_MODEL",   TypeInt, x_p(TRACE_TYPE_MODEL));
	// animation operations
	add_const("MOVE_OP_SET",         TypeInt, x_p(MOVE_OP_SET));
	add_const("MOVE_OP_SET_NEW_KEYED", TypeInt, x_p(MOVE_OP_SET_NEW_KEYED));
	add_const("MOVE_OP_SET_OLD_KEYED", TypeInt, x_p(MOVE_OP_SET_OLD_KEYED));
	add_const("MOVE_OP_ADD_1_FACTOR",  TypeInt, x_p(MOVE_OP_ADD_1_FACTOR));
	add_const("MOVE_OP_MIX_1_FACTOR",  TypeInt, x_p(MOVE_OP_MIX_1_FACTOR));
	add_const("MOVE_OP_MIX_2_FACTOR",  TypeInt, x_p(MOVE_OP_MIX_2_FACTOR));
	

#if _X_ALLOW_X_
	AllowXContainer = true;
#endif
}

};
