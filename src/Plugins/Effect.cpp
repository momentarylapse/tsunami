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
#include "../Stuff/Log.h"
#include "PluginManager.h"

Effect::Effect()
{
	usable = false;
	plugin = NULL;
	only_on_selection = false;
}

Effect::Effect(Plugin *p)
{
	usable = false;
	if (p)
		usable = p->usable;
	plugin = p;
	only_on_selection = false;
}

void Effect::ExportData()
{
	make_usable();
	if (usable)
		plugin->ExportData(param);
}

void Effect::ImportData()
{
	make_usable();
	if (usable)
		plugin->ImportData(param);
}

void Effect::ExportState()
{
	msg_db_r("Effect.ExportState", 1);
	make_usable();
	if (usable)
		if (plugin->state)
			memcpy(state, plugin->state, plugin->state_type->Size);
	msg_db_l(1);
}

void Effect::ImportState()
{
	msg_db_r("Effect.ImportState", 1);
	make_usable();
	if (usable)
		if (plugin->state)
			memcpy(plugin->state, state, plugin->state_type->Size);
	msg_db_l(1);
}

void Effect::Prepare()
{
	msg_db_r("Effect.Prepare", 1);
	make_usable();
	if (usable){
		if (plugin->state){
			ImportData();
			state = new char[plugin->state_type->Size];
			// TODO (init)
			plugin->ResetState();
			ExportState();
		}
	}else{
		tsunami->log->Error(GetError());
	}
	msg_db_l(1);
}

void Effect::CleanUp()
{
	msg_db_r("Effect.CleanUp", 1);
	make_usable();
	if (usable){
		if (plugin->state){
			ImportState();
			plugin->ResetState();
			// TODO (clear)
			ExportState();
			delete[]((char*)state);
		}
	}
	msg_db_l(1);
}



void Effect::make_usable()
{
	if (!plugin){
		plugin = tsunami->plugins->GetPlugin(name);
		if (plugin)
			usable = plugin->usable;
	}
}

string Effect::GetError()
{
	if (plugin)
		return plugin->GetError();
	return format(_("Effekt nicht ladbar: \"%s\""), name.c_str());
}

void Effect::Apply(BufferBox &buf, Track *t, bool log_error)
{
	msg_db_r("Effect.Apply", 1);

	make_usable();
	if (usable){
		// run
		plugin->ResetData();
		ImportData();
		ImportState();
		if (plugin->type == Plugin::TYPE_EFFECT)
			plugin->f_process_track(&buf, t, 0);
		ExportState();
		t->root->UpdateSelection();
	}else{
		if (log_error)
			tsunami->log->Error(_("Beim Anwenden eines Effekts: ") + GetError());
	}

	msg_db_l(1);
}
