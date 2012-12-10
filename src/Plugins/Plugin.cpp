/*
 * Plugin.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Plugin.h"
#include "../Tsunami.h"
#include "../Action/Track/ActionTrackEditBuffer.h"


// -> PluginManager.cpp
void GlobalRemoveSliders(CHuiWindow *win);

Plugin::Plugin(const string &_filename)
{
	filename = _filename;
	index = -1;

	// load + compile
	s = LoadScript(filename);

	// NULL if error ...
	f_reset = (void_func*)s->MatchFunction("ResetData", "void", 0);
	f_data2dialog = (void_func*)s->MatchFunction("DataToDialog", "void", 0);
	f_configure = (void_func*)s->MatchFunction("Configure", "void", 0);
	f_reset_state = (void_func*)s->MatchFunction("ResetState", "void", 0);
	f_process_track = (process_track_func*)s->MatchFunction("ProcessTrack", "void", 3, "BufferBox", "Track", "int");

	type = f_process_track ? TYPE_EFFECT : TYPE_OTHER;

	data = NULL;
	data_type = NULL;
	state = NULL;
	state_type = NULL;

	foreachi(sLocalVariable &v, s->pre_script->RootOfAllEvil.Var, i)
		if (v.Type->Name == "PluginData"){
			data = s->g_var[i];
			data_type = v.Type;
		}else if (v.Type->Name == "PluginState"){
			state = s->g_var[i];
			state_type = v.Type;
		}
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
	msg_db_r("Plugin.ResetState", 1);
	if (f_reset_state)
		f_reset_state();
	msg_db_l(1);
}

bool Plugin::Configure(bool previewable)
{
	msg_db_r("Plugin.Configure", 1);
	if (f_configure){
		tsunami->plugins->PluginAddPreview = previewable;
		f_configure();
		GlobalRemoveSliders(NULL);
		msg_db_l(1);
		return !tsunami->plugins->PluginCancelled;
	}else{
		tsunami->log->Info(_("Dieser Effekt ist nicht konfigurierbar."));
	}
	msg_db_l(1);
	return true;
}

void Plugin::DataToDialog()
{
	if (f_data2dialog)
		f_data2dialog();
}

void Plugin::ProcessTrack(Track *t, int level_no, Range r)
{
	if (!f_process_track)
		return;
	msg_db_r("PluginProcessTrack", 1);
	BufferBox buf = t->GetBuffers(level_no, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, level_no, r);
	f_process_track(&buf, t, level_no);
	t->root->Execute(a);
	msg_db_l(1);
}
