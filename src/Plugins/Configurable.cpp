/*
 * Configurable.cpp
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */


#include "Configurable.h"
#include "../Tsunami.h"
#include "../lib/script/script.h"
#include "PluginManager.h"
#include "../Stuff/Log.h"


void PluginData::__init__()
{
	new(this) PluginData;
}

void PluginData::__delete__()
{
}


Array<Script::ClassElement> get_unique_elements(Script::Type *t)
{
	Array<Script::ClassElement> r;
	int pos = -1;
	foreach(Script::ClassElement &e, t->element)
		if ((e.offset > pos) && (e.name[0] != '-')){
			r.add(e);
			pos = e.offset;
		}
	return r;
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
	}else if (type == Script::TypeString){
		r += "\"" + str_escape(*(string*)v) + "\"";
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
		Array<Script::ClassElement> e = get_unique_elements(type);
		r += "(";
		for(int i=0;i<e.num;i++){
			if (i > 0)
				r += " ";
			r += var_to_string(e[i].type, &v[e[i].offset]);
		}
		r += ")";
	}
	return r;
}

string get_next(const string &var_temp, int &pos)
{
	int start = pos;
	bool in_string = false;
	for (int i=start;i<var_temp.num;i++){
		if ((i == start) && (var_temp[i] == '"')){
			in_string = true;
		}else if (in_string){
			if (var_temp[i] == '\\'){
				i ++;
			}else if (var_temp[i] == '"'){
				pos = i + 1;
				return str_unescape(var_temp.substr(start + 1, i - start - 1));
			}
		}else if ((var_temp[i] == ' ') || (var_temp[i] == ']') || (var_temp[i] == ')') || (var_temp[i] == '[') || (var_temp[i] == '(')){
			pos = i;
			return var_temp.substr(start, i - start);
		}
	}
	return var_temp.substr(start, -1);
}

void var_from_string(Script::Type *type, char *v, const string &s, int &pos)
{
	if (pos >= s.num)
		return;
	if (type == Script::TypeInt){
		*(int*)v = get_next(s, pos)._int();
	}else if (type == Script::TypeChar){
		*(char*)v = get_next(s, pos)._int();
	}else if (type == Script::TypeFloat){
		*(float*)v = get_next(s, pos)._float();
	}else if (type == Script::TypeBool){
		*(bool*)v = (get_next(s, pos) == "true");
	}else if (type == Script::TypeString){
		*(string*)v = get_next(s, pos);
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
		Array<Script::ClassElement> e = get_unique_elements(type);
		pos ++; // '('
		for(int i=0;i<e.num;i++){
			if (i > 0)
				pos ++; // ' '
			var_from_string(e[i].type, &v[e[i].offset], s, pos);
		}
		pos ++; // ')'
	}
}

void Configurable::__init__()
{
	new(this) Configurable;
}

void Configurable::__delete__()
{
}

PluginData *Configurable::get_config()
{
	Script::Type *type = Script::GetDynamicType(this);
	if (!type)
		return NULL;
	foreach(Script::ClassElement &e, type->element)
		if ((e.name == "config") && (e.type->GetRoot()->name == "PluginData")){
			PluginData *config = (PluginData*)((char*)this + e.offset);
			config->type = e.type;
			return config;
		}
	return NULL;
}

PluginData *Configurable::get_state()
{
	Script::Type *type = Script::GetDynamicType(this);
	if (!type)
		return NULL;
	foreach(Script::ClassElement &e, type->element)
		if ((e.name == "state") && (e.type->GetRoot()->name == "PluginData")){
			PluginData *state = (PluginData*)((char*)this + e.offset);
			state->type = e.type;
			return state;
		}
	return NULL;
}

string Configurable::ConfigToString()
{
	msg_db_f("Configurable.ConfigToString", 1);

	PluginData *config = get_config();
	if (!config)
		return "";

	return var_to_string(config->type, (char*)config);
}

void Configurable::ConfigFromString(const string &param)
{
	msg_db_f("Configurable.ConfigFromString", 1);

	PluginData *config = get_config();
	if (!config)
		return;

	int pos = 0;
	var_from_string(config->type, (char*)config, param, pos);
}


// default version of ResetConfig()
//   try to execute   Configurable.config.reset()
void Configurable::ResetConfig()
{
	msg_db_f("Configurable.ResetConfig", 1);

	PluginData *config = get_config();
	if (!config)
		return;
	config->reset();
}

// default version of ResetState()
//   try to execute   Configurable.state.reset()
void Configurable::ResetState()
{
	msg_db_f("Configurable.ResetState", 1);

	PluginData *state = get_state();
	if (!state)
		return;
	state->reset();
}



