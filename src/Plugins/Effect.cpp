/*
 * Effect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Effect.h"

#include "Plugin.h"
#include "../Tsunami.h"
#include "../lib/script/script.h"
#include "../lib/math/math.h"
#include "../Stuff/Log.h"
#include "PluginManager.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"

// -> PluginManager.cpp
void GlobalRemoveSliders(HuiPanel *panel);

Effect::Effect()
{
	usable = true;
	plugin = NULL;
	only_on_selection = false;
}

Effect::Effect(Plugin *p)
{
	usable = true;
	plugin = p;
	only_on_selection = false;
}

Effect::~Effect()
{
}

void Effect::__init__()
{
	new(this) Effect;
}

void Effect::__delete__()
{
}

/*void make_fx(Effect *fx)
{
	foreachi(Script::Variable &v, fx->plugin->s->syntax->RootOfAllEvil.var, i)
		if (v.type->name == "PluginData"){
			fx->data = fx->plugin->s->g_var[i];
			fx->data_type = v.type;
		}
}*/

void Effect::Prepare()
{
	msg_db_f("Effect.Prepare", 1);
	ResetState();
	if (!usable)
		tsunami->log->Error(GetError());
}

string Effect::GetError()
{
	if (plugin)
		return plugin->GetError();
	return format(_("Effekt nicht ladbar: \"%s\""), name.c_str());
}

void Effect::Apply(BufferBox &buf, Track *t, bool log_error)
{
	msg_db_f("Effect.Apply", 1);

	// run
	tsunami->plugin_manager->context.set(t, 0, buf.range());
	ProcessTrack(&buf);
	t->root->UpdateSelection();

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			tsunami->log->Error(_("Beim Anwenden eines Effekts: ") + GetError());
	}
}

void Effect::WriteConfigToFile(const string &fav_name)
{
	msg_db_f("Effect.ConfigDataToFile", 1);
	ConfigToString();
	dir_create(HuiAppDirectory + "Favorites/");
	CFile *f = FileCreate(HuiAppDirectory + "Favorites/Effect/" + name + "___" + fav_name);
	f->WriteStr(ConfigToString());
	delete(f);
}

void Effect::LoadConfigFromFile(const string &fav_name)
{
	msg_db_f("Effect.LoadConfigFromFile", 1);
	CFile *f = FileOpen(HuiAppDirectory + "Favorites/Effect/" + name + "___" + fav_name);
	if (!f)
		return;
	ConfigFromString(f->ReadStr());
	delete(f);
}


bool Effect::DoConfigure(bool previewable)
{
	msg_db_f("Effect.DoConfigure", 1);
	tsunami->plugin_manager->cur_effect = this;
	tsunami->plugin_manager->PluginAddPreview = previewable;
	tsunami->plugin_manager->PluginCancelled = false;
	Configure();
	GlobalRemoveSliders(NULL);
	return !tsunami->plugin_manager->PluginCancelled;
	//tsunami->log->Info(_("Dieser Effekt ist nicht konfigurierbar."));
	//return true;
}



void Effect::DoProcessTrack(Track *t, int level_no, const Range &r)
{
	msg_db_f("Effect.DoProcessTrack", 1);

	tsunami->plugin_manager->context.set(t, level_no, r);

	BufferBox buf = t->GetBuffers(level_no, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, level_no, r);
	ProcessTrack(&buf);
	t->root->Execute(a);
}


Effect *CreateEffect(const string &name)
{
	Effect *f = tsunami->plugin_manager->LoadEffect(name);
	if (f){
		f->name = name;
		f->ResetConfig();
		return f;
	}
	f = new Effect;
	f->name = name;
	f->plugin = tsunami->plugin_manager->GetPlugin(name);
	if (f->plugin){
		f->usable = f->plugin->usable;
	}
	return f;
}
