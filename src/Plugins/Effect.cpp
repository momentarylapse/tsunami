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
void GlobalRemoveSliders(HuiWindow *win);

string var_to_string(Script::Type *type, char *v);
void var_from_string(Script::Type *type, char *v, const string &s, int &pos);
void try_read_element(EffectParam &p, Script::ClassElement *e, char *v);
void try_write_element(EffectParam *p, Script::ClassElement *e, char *v);

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

void Effect::ExportConfig()
{
	msg_db_f("Effect.ExportConfig", 1);

	param.clear();
	Script::Type *type = Script::GetDynamicType(this);
	if (type){
		foreach(Script::ClassElement &e, type->element)
			if (e.name == "config"){
				param.resize(e.type->element.num);
				foreachi(Script::ClassElement &ee, e.type->element, j){
					char *p = (char*)this;
					param[j].value = var_to_string(ee.type, &p[e.offset + ee.offset]);
				}
			}
	}
}

void Effect::ImportConfig()
{
	msg_db_f("Effect.ImportConfig", 1);

	Script::Type *type = Script::GetDynamicType(this);
	if (type){
		foreach(Script::ClassElement &e, type->element)
			if (e.name == "config"){
				param.resize(e.type->element.num);
				foreachi(Script::ClassElement &ee, e.type->element, j){
					char *p = (char*)this;
					int pos = 0;
					var_from_string(ee.type, &p[e.offset + ee.offset], param[j].value, pos);
				}
			}
	}
}

void Effect::Prepare()
{
	msg_db_f("Effect.Prepare", 1);
	ImportConfig();
	ResetState();
	if (!usable)
		tsunami->log->Error(GetError());
}

void Effect::CleanUp()
{
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
	ResetConfig();
	ImportConfig();
	tsunami->plugin_manager->context.set(t, 0, buf.range());
	ProcessTrack(&buf);
	t->root->UpdateSelection();

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			tsunami->log->Error(_("Beim Anwenden eines Effekts: ") + GetError());
	}
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

void Effect::WriteConfigToFile(const string &name)
{
	msg_db_f("Effect.ConfigDataToFile", 1);
	ExportConfig();
	dir_create(HuiAppDirectory + "Favorites/");
	CFile *f = FileCreate(HuiAppDirectory + "Plugins/Favorites/" + plugin->s->Filename.basename() + "___" + name);
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
	delete(f);
}

void Effect::LoadConfigFromFile(const string &name)
{
	msg_db_f("Effect.LoadConfigFromFile", 1);
	CFile *f = FileOpen(HuiAppDirectory + "Favorites/" + plugin->s->Filename.basename() + "___" + name);
	if (!f)
		return;

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
	ImportConfig();

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
