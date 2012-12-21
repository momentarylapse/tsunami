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
#include "../Stuff/Log.h"
#include "../Action/Track/ActionTrackEditBuffer.h"


// -> PluginManager.cpp
void GlobalRemoveSliders(CHuiWindow *win);

Plugin::Plugin(const string &_filename)
{
	filename = _filename;
	index = -1;
	usable = false;
	data = NULL;
	data_type = NULL;
	state = NULL;
	state_type = NULL;
	f_reset = NULL;
	f_data2dialog = NULL;
	f_configure = NULL;
	f_reset_state = NULL;
	f_process_track = NULL;

	// load + compile
	s = LoadScript(filename);

	if (s){

		usable = !s->Error;

		if (usable){
			f_reset = (void_func*)s->MatchFunction("ResetData", "void", 0);
			f_data2dialog = (void_func*)s->MatchFunction("DataToDialog", "void", 0);
			f_configure = (void_func*)s->MatchFunction("Configure", "void", 0);
			f_reset_state = (void_func*)s->MatchFunction("ResetState", "void", 0);
			f_process_track = (process_track_func*)s->MatchFunction("ProcessTrack", "void", 3, "BufferBox", "Track", "int");

			type = f_process_track ? TYPE_EFFECT : TYPE_OTHER;

			foreachi(sLocalVariable &v, s->pre_script->RootOfAllEvil.Var, i)
				if (v.Type->Name == "PluginData"){
					data = s->g_var[i];
					data_type = v.Type;
				}else if (v.Type->Name == "PluginState"){
					state = s->g_var[i];
					state_type = v.Type;
				}
		}
	}
}

string Plugin::GetError()
{
	if (s)
		return format(_("Fehler in  Script-Datei: \"%s\"\n%s\n%s"), s->pre_script->Filename.c_str(), s->ErrorMsgExt[0].c_str(), s->ErrorMsgExt[1].c_str());
	return format(_("Fehler in  Script-Datei: \"%s\""), filename.c_str());
}

void try_write_primitive_element(string &var_temp, sType *t, char *v)
{
	if (t == TypeInt)
		var_temp += i2s(*(int*)v);
	else if (t == TypeChar)
		var_temp += i2s(*(char*)v);
	else if (t == TypeFloat)
		var_temp += f2s(*(float*)v, 6);
	else if (t == TypeBool)
		var_temp += (*(bool*)v) ? "true" : "false";
	else if (t == TypeVector)
		var_temp += format("(%f %f %f)", *(float*)v, ((float*)v)[1], ((float*)v)[2]);
	else if (t == TypeComplex)
		var_temp += format("(%f %f)", *(float*)v, ((float*)v)[1]);
	else
		var_temp += "-------";
}

void try_write_element(EffectParam *p, sClassElement *e, char *v)
{
	p->name = e->Name;
	p->type = e->Type->Name;
	p->value = "";
	if (e->Type->IsArray){
		p->value += "[";
		for (int i=0;i<e->Type->ArrayLength;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->Type->SubType, &v[e->Offset + i * e->Type->SubType->Size]);
		}
		p->value += "]";
	}else if (e->Type->IsSuperArray){
		DynamicArray *a = (DynamicArray*)&v[e->Offset];
		p->value += format("[%d ", a->num);
		for (int i=0;i<a->num;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->Type->SubType, &(((char*)a->data)[i * e->Type->SubType->Size]));
		}
		p->value += "]";
	}else
		try_write_primitive_element(p->value, e->Type, &v[e->Offset]);
}

string get_next(const string &var_temp, int &pos)
{
	int start = pos;
	for (int i=pos;i<var_temp.num;i++){
		if ((var_temp[i] != ' ') && (var_temp[i] != ']') && (var_temp[i] != ')') && (var_temp[i] != '[') && (var_temp[i] != '(')){
			start = i;
			break;
		}
	}
	for (int i=start;i<var_temp.num;i++){
		if ((var_temp[i] == ' ') || (var_temp[i] == ']') || (var_temp[i] == ')') || (var_temp[i] == '[') || (var_temp[i] == '(')){
			pos = i + 1;
			return var_temp.substr(start, i - start);
			break;
		}
	}
	return var_temp.substr(start, -1);
}

void try_read_primitive_element(const string &var_temp, int &pos, sType *t, char *v)
{
	if (t == TypeInt)
		*(int*)v = s2i(get_next(var_temp, pos));
	else if (t == TypeChar)
		*(char*)v = s2i(get_next(var_temp, pos));
	else if (t == TypeFloat)
		*(float*)v = s2f(get_next(var_temp, pos));
	else if (t == TypeComplex){
		((complex*)v)->x = s2f(get_next(var_temp, pos));
		((complex*)v)->y = s2f(get_next(var_temp, pos));
	}else if (t == TypeVector){
		((vector*)v)->x = s2f(get_next(var_temp, pos));
		((vector*)v)->y = s2f(get_next(var_temp, pos));
		((vector*)v)->z = s2f(get_next(var_temp, pos));
	}else if (t == TypeBool)
		*(bool*)v = (get_next(var_temp, pos) == "true");
}

void try_read_element(EffectParam &p, sClassElement *e, char *v)
{
	int pos = 0;
	if (e->Type->IsArray){
		for (int i=0;i<e->Type->ArrayLength;i++)
			try_read_primitive_element(p.value, pos, e->Type->SubType, &v[e->Offset + i * e->Type->SubType->Size]);
	}else if (e->Type->IsSuperArray){
		DynamicArray *a = (DynamicArray*)&v[e->Offset];
		int num = s2i(get_next(p.value, pos));
		a->resize(num);
		for (int i=0;i<num;i++)
			try_read_primitive_element(p.value, pos, e->Type->SubType, &(((char*)a->data)[i * e->Type->SubType->Size]));
	}else
		try_read_primitive_element(p.value, pos, e->Type, &v[e->Offset]);
}

void Plugin::ExportData(Array<EffectParam> &param)
{
	msg_db_r("Plugin.ExportData", 1);
	param.clear();
	if (data){
		param.resize(data_type->Element.num);
		foreachi(sClassElement &e, data_type->Element, j)
			try_write_element(&param[j], &e, (char*)data);
	}
	msg_db_l(1);
}

void Plugin::ImportData(Array<EffectParam> &param)
{
	msg_db_r("Plugin.ImportData", 1);
	if (data){
		foreach(sClassElement &e, data_type->Element)
			foreach(EffectParam &p, param)
				if ((e.Name == p.name) && (e.Type->Name == p.type))
					try_read_element(p, &e, (char*)data);
	}
	msg_db_l(1);
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
		tsunami->plugins->cur_plugin = this;
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

void Plugin::WriteDataToFile(const string &name)
{
	if (!usable)
		return;
	msg_db_r("Plugin.WriteDataToFile", 1);
	Array<EffectParam> param;
	ExportData(param);
	dir_create(HuiAppDirectory + "Plugins/");
	dir_create(HuiAppDirectory + "Plugins/Favorites/");
	CFile *f = CreateFile(HuiAppDirectory + "Plugins/Favorites/" + s->pre_script->Filename.basename() + "___" + name);
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteComment("// Data");
	f->WriteInt(param.num);
	foreach(EffectParam &p, param){
		f->WriteStr(p.name);
		f->WriteStr(p.type);
		f->WriteStr(p.value);
	}
	f->WriteStr("#");
	FileClose(f);
	msg_db_l(1);
}

void Plugin::LoadDataFromFile(const string &name)
{
	if (!usable)
		return;
	msg_db_r("Plugin.LoadDataFromFile", 1);
	CFile *f = OpenFile(HuiAppDirectory + "Plugins/Favorites/" + s->pre_script->Filename.basename() + "___" + name);
	if (!f){
		msg_db_l(1);
		return;
	}

	Array<EffectParam> param;

	f->ReadInt();
	f->ReadInt();
	f->ReadComment();
	int num = f->ReadInt();
	param.resize(num);
	foreach(EffectParam &p, param){
		p.name = f->ReadStr();
		p.type = f->ReadStr();
		p.value = f->ReadStr();
	}
	ImportData(param);

	FileClose(f);
	msg_db_l(1);
}
