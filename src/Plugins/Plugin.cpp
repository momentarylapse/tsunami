/*
 * Plugin.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Plugin.h"
#include "../Tsunami.h"
#include "../lib/script/script.h"
#include "PluginManager.h"
#include "Effect.h"
#include "../Stuff/Log.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"


// -> PluginManager.cpp
void GlobalRemoveSliders(HuiWindow *win);

Plugin::Plugin(const string &_filename)
{
	s = NULL;
	filename = _filename;
	index = -1;
	usable = false;
	f_reset = NULL;
	f_data2dialog = NULL;
	f_configure = NULL;
	f_reset_state = NULL;
	f_process_track = NULL;

	// load + compile
	try{
		s = Script::Load(filename);

		usable = true;

		f_reset = (void_func*)s->MatchFunction("ResetData", "void", 0);
		f_data2dialog = (void_func*)s->MatchFunction("DataToDialog", "void", 0);
		f_configure = (void_func*)s->MatchFunction("Configure", "void", 0);
		f_reset_state = (void_func*)s->MatchFunction("ResetState", "void", 0);
		f_process_track = (process_track_func*)s->MatchFunction("ProcessTrack", "void", 1, "BufferBox");

		type = f_process_track ? TYPE_EFFECT : TYPE_OTHER;
	}catch(Script::Exception &e){
		error_message = e.message;
	}
}

string Plugin::GetError()
{
	return format(_("Fehler in  Script-Datei: \"%s\"\n%s"), filename.c_str(), error_message.c_str());
}

void Plugin::ResetData()
{
	msg_db_r("Plugin.ResetData", 1);
	if (f_reset)
		f_reset();
	msg_db_l(1);
}

void Plugin::ResetState()
{
	msg_db_f("Plugin.ResetState", 1);
	if (f_reset_state)
		f_reset_state();
}

bool Plugin::Configure(bool previewable)
{
	msg_db_f("Plugin.Configure", 1);
	if (f_configure){
		tsunami->plugin_manager->cur_plugin = this;
		tsunami->plugin_manager->PluginAddPreview = previewable;
		f_configure();
		GlobalRemoveSliders(NULL);
		return !tsunami->plugin_manager->PluginCancelled;
	}else{
		tsunami->log->Info(_("Dieser Effekt ist nicht konfigurierbar."));
	}
	return true;
}

void Plugin::DataToDialog()
{
	if (f_data2dialog)
		f_data2dialog();
}

void Plugin::ProcessTrack(Track *t, int level_no, const Range &r)
{
	if (!f_process_track)
		return;
	msg_db_f("PluginProcessTrack", 1);

	tsunami->plugin_manager->context.set(t, level_no, r);

	BufferBox buf = t->GetBuffers(level_no, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, level_no, r);
	f_process_track(&buf);
	t->root->Execute(a);
}
