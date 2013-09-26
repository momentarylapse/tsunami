/*
 * Plugin.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Plugin.h"
#include "../Tsunami.h"
#include "../lib/script/script.h"
#include "../lib/math/math.h"
#include "PluginManager.h"
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
	try{
		s = Script::Load(filename);

		usable = true;

		f_reset = (void_func*)s->MatchFunction("ResetData", "void", 0);
		f_data2dialog = (void_func*)s->MatchFunction("DataToDialog", "void", 0);
		f_configure = (void_func*)s->MatchFunction("Configure", "void", 0);
		f_reset_state = (void_func*)s->MatchFunction("ResetState", "void", 0);
		f_process_track = (process_track_func*)s->MatchFunction("ProcessTrack", "void", 1, "BufferBox");

		type = f_process_track ? TYPE_EFFECT : TYPE_OTHER;

		foreachi(Script::Variable &v, s->syntax->RootOfAllEvil.var, i)
			if (v.type->name == "PluginData"){
				data = s->g_var[i];
				data_type = v.type;
			}else if (v.type->name == "PluginState"){
				state = s->g_var[i];
				state_type = v.type;
			}
	}catch(Script::Exception &e){
		error_message = e.message;
	}
}

string Plugin::GetError()
{
	return format(_("Fehler in  Script-Datei: \"%s\"\n%s"), filename.c_str(), error_message.c_str());
}

void try_write_primitive_element(string &var_temp, Script::Type *t, char *v)
{
	if (t == Script::TypeInt)
		var_temp += i2s(*(int*)v);
	else if (t == Script::TypeChar)
		var_temp += i2s(*(char*)v);
	else if (t == Script::TypeFloat)
		var_temp += f2s(*(float*)v, 6);
	else if (t == Script::TypeBool)
		var_temp += (*(bool*)v) ? "true" : "false";
	else if (t == Script::TypeVector)
		var_temp += format("(%f %f %f)", *(float*)v, ((float*)v)[1], ((float*)v)[2]);
	else if (t == Script::TypeComplex)
		var_temp += format("(%f %f)", *(float*)v, ((float*)v)[1]);
	else
		var_temp += "-------";
}

string var_to_string(Script::Type *type, char *v)
{
	string r;
	if (type == Script::TypeInt){
		r += i2s(*(int*)v);
	}else if (type == Script::TypeChar){
		r += i2s(*(char*)v);
	}else if (type == Script::TypeFloat){
		r += f2s(*(float*)v, 6);
	}else if (type == Script::TypeBool){
		r += (*(bool*)v) ? "true" : "false";
	}else if (type->is_array){
		r += "[";
		for (int i=0;i<type->array_length;i++){
			if (i > 0)
				r += " ";
			r += var_to_string(type->parent, &v[i * type->parent->size]);
		}
		r += "]";
	}else if (type->is_super_array){
		DynamicArray *a = (DynamicArray*)v;
		r += "[";
		for (int i=0;i<a->num;i++){
			if (i > 0)
				r += " ";
			r += var_to_string(type->parent, &(((char*)a->data)[i * type->parent->size]));
		}
		r += "]";
	}else{
		r += "(";
		for(int i=0;i<type->element.num;i++){
			if (i > 0)
				r += " ";
			r += var_to_string(type->element[i].type, &v[type->element[i].offset]);
		}
		r += ")";
	}
	return r;
}

void try_write_element(EffectParam *p, Script::ClassElement *e, char *v)
{
	p->name = e->name;
	p->type = e->type->name;
	p->value = "";
	if (e->type->is_array){
		p->value += "[";
		for (int i=0;i<e->type->array_length;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->type->parent, &v[e->offset + i * e->type->parent->size]);
		}
		p->value += "]";
	}else if (e->type->is_super_array){
		DynamicArray *a = (DynamicArray*)&v[e->offset];
		p->value += format("[%d ", a->num);
		for (int i=0;i<a->num;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->type->parent, &(((char*)a->data)[i * e->type->parent->size]));
		}
		p->value += "]";
	}else
		try_write_primitive_element(p->value, e->type, &v[e->offset]);
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

string get_next2(const string &var_temp, int &pos)
{
	int start = pos;
	for (int i=start;i<var_temp.num;i++){
		if ((var_temp[i] == ' ') || (var_temp[i] == ']') || (var_temp[i] == ')') || (var_temp[i] == '[') || (var_temp[i] == '(')){
			pos = i;
			return var_temp.substr(start, i - start);
			break;
		}
	}
	return var_temp.substr(start, -1);
}

void var_from_string(Script::Type *type, char *v, const string &s, int &pos)
{
	if (pos >= s.num)
		return;
	if (type == Script::TypeInt){
		*(int*)v = get_next2(s, pos)._int();
	}else if (type == Script::TypeChar){
		get_next2(s, pos);
		*(char*)v = 'x';
	}else if (type == Script::TypeFloat){
		*(float*)v = get_next2(s, pos)._float();
	}else if (type == Script::TypeBool){
		*(bool*)v = (get_next2(s, pos) == "true");
	}else if (type->is_array){
		pos ++; // '['
		for (int i=0;i<type->array_length;i++){
			if (i > 0)
				pos ++; // ' '
			var_from_string(type->parent, &v[i * type->parent->size], s, pos);
		}
		pos ++; // ']'
	}else if (type->is_super_array){
		pos ++; // '['
		DynamicArray *a = (DynamicArray*)v;
		a->clear(); // todo...
		while (true){
			if ((s[pos] == ']') || (pos >= s.num))
				break;
			if (a->num > 0)
				pos ++; // ' '
			a->resize(a->num + 1);
			var_from_string(type->parent, &(((char*)a->data)[(a->num - 1) * type->parent->size]), s, pos);
		}
		pos ++; // ']'
	}else{
		pos ++; // '('
		for(int i=0;i<type->element.num;i++){
			if (i > 0)
				pos ++; // ' '
			var_from_string(type->element[i].type, &v[type->element[i].offset], s, pos);
		}
		pos ++; // ')'
	}
}

void try_read_primitive_element(const string &var_temp, int &pos, Script::Type *t, char *v)
{
	if (t == Script::TypeInt)
		*(int*)v = s2i(get_next(var_temp, pos));
	else if (t == Script::TypeChar)
		*(char*)v = s2i(get_next(var_temp, pos));
	else if (t == Script::TypeFloat)
		*(float*)v = s2f(get_next(var_temp, pos));
	else if (t == Script::TypeComplex){
		((complex*)v)->x = s2f(get_next(var_temp, pos));
		((complex*)v)->y = s2f(get_next(var_temp, pos));
	}else if (t == Script::TypeVector){
		((vector*)v)->x = s2f(get_next(var_temp, pos));
		((vector*)v)->y = s2f(get_next(var_temp, pos));
		((vector*)v)->z = s2f(get_next(var_temp, pos));
	}else if (t == Script::TypeBool)
		*(bool*)v = (get_next(var_temp, pos) == "true");
}

void try_read_element(EffectParam &p, Script::ClassElement *e, char *v)
{
	int pos = 0;
	if (e->type->is_array){
		for (int i=0;i<e->type->array_length;i++)
			try_read_primitive_element(p.value, pos, e->type->parent, &v[e->offset + i * e->type->parent->size]);
	}else if (e->type->is_super_array){
		DynamicArray *a = (DynamicArray*)&v[e->offset];
		int num = s2i(get_next(p.value, pos));
		a->resize(num);
		for (int i=0;i<num;i++)
			try_read_primitive_element(p.value, pos, e->type->parent, &(((char*)a->data)[i * e->type->parent->size]));
	}else
		try_read_primitive_element(p.value, pos, e->type, &v[e->offset]);
}

void Plugin::ExportData(Array<EffectParam> &param)
{
	msg_db_f("Plugin.ExportData", 1);
	param.clear();
	if (data){
		param.resize(data_type->element.num);
		foreachi(Script::ClassElement &e, data_type->element, j)
			try_write_element(&param[j], &e, (char*)data);
	}
}

void Plugin::ImportData(Array<EffectParam> &param)
{
	msg_db_f("Plugin.ImportData", 1);
	if (data){
		foreach(Script::ClassElement &e, data_type->element)
			foreach(EffectParam &p, param)
				if ((e.name == p.name) && (e.type->name == p.type))
					try_read_element(p, &e, (char*)data);
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

void Plugin::WriteDataToFile(const string &name)
{
	if (!usable)
		return;
	msg_db_f("Plugin.WriteDataToFile", 1);
	Array<EffectParam> param;
	ExportData(param);
	dir_create(HuiAppDirectory + "Favorites/");
	CFile *f = FileCreate(HuiAppDirectory + "Plugins/Favorites/" + s->Filename.basename() + "___" + name);
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
}

void Plugin::LoadDataFromFile(const string &name)
{
	if (!usable)
		return;
	msg_db_f("Plugin.LoadDataFromFile", 1);
	CFile *f = FileOpen(HuiAppDirectory + "Favorites/" + s->Filename.basename() + "___" + name);
	if (!f)
		return;

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
}
