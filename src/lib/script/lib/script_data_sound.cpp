#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"


#ifdef _X_USE_SOUND_
	#include "../../sound/sound.h"
#endif


namespace Script{

#ifdef _X_USE_SOUND_
	#define sound_p(p)		(void*)p
#else
	#define sound_p(p)		NULL
#endif

extern Type *TypeFloatList;

void SIAddPackageSound()
{
	set_cur_package("sound");
	
	// sound
	add_func("SoundEmit",									TypeInt,	sound_p(&SoundEmit));
		func_add_param("filename",	TypeString);
		func_add_param("pos",		TypeVector);
		func_add_param("r_min",		TypeFloat);
		func_add_param("r_max",		TypeFloat);
		func_add_param("speed",		TypeFloat);
		func_add_param("volume",	TypeFloat);
		func_add_param("loop",		TypeBool);
	add_func("SoundLoad",									TypeInt,	sound_p(&SoundLoad));
		func_add_param("filename",	TypeString);
	add_func("SoundSetData",							TypeVoid,	sound_p(&SoundSetData));
		func_add_param("index",		TypeInt);
		func_add_param("pos",		TypeVector);
		func_add_param("vel",		TypeVector);
		func_add_param("r_min",		TypeFloat);
		func_add_param("r_max",		TypeFloat);
		func_add_param("speed",		TypeFloat);
		func_add_param("volume",	TypeFloat);
	add_func("SoundPlay",							TypeVoid,	sound_p(&SoundPlay));
		func_add_param("index",		TypeInt);
		func_add_param("loop",		TypeBool);
	add_func("SoundStop",							TypeVoid,	sound_p(&SoundStop));
		func_add_param("index",		TypeInt);
	add_func("SoundPause",							TypeVoid,	sound_p(&SoundPause));
		func_add_param("index",		TypeInt);
		func_add_param("pause",		TypeBool);
	add_func("SoundDelete",									TypeVoid,	sound_p(&SoundDelete));
		func_add_param("index",		TypeInt);
	// music
	add_func("MusicLoad",									TypeInt,	sound_p(&MusicLoad));
		func_add_param("filename",		TypeString);
	add_func("MusicPlay",									TypeInt,	sound_p(&MusicPlay));
		func_add_param("index",		TypeInt);
		func_add_param("loop",		TypeBool);
	add_func("MusicStop",									TypeInt,	sound_p(&MusicStop));
		func_add_param("index",		TypeInt);
	add_func("MusicPause",									TypeInt,	sound_p(&MusicPause));
		func_add_param("index",		TypeInt);
		func_add_param("pause",		TypeBool);
	add_func("MusicSetRate",							TypeInt,	sound_p(&MusicSetRate));
		func_add_param("index",		TypeInt);
		func_add_param("rate",		TypeFloat);
	// sound creation
	add_func("SoundSave",		TypeVoid, 		sound_p(&SoundSaveFile));
		func_add_param("filename",		TypeString);
		func_add_param("buf_r",			TypeFloatList);
		func_add_param("buf_l",			TypeFloatList);
		func_add_param("freq",			TypeInt);
		func_add_param("channels",		TypeInt);
		func_add_param("bits",			TypeInt);
	
	add_ext_var("VolumeMusic",		TypeFloat,		sound_p(&VolumeMusic));
	add_ext_var("VolumeSounds",		TypeFloat,		sound_p(&VolumeSound));
}

};
