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
#include "../View/Helper/Slider.h"


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

Configurable::Configurable(const string &observable_name) :
	Observable(observable_name)
{
}

void Configurable::__init__()
{
	new(this) Configurable("");
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

HuiPanel *Configurable::CreatePanel()
{
	return NULL;
}

struct AutoConfigData
{
	string name, unit;
	float min, max, step, factor;
	float *value;
	AutoConfigData()
	{
		min = -100000000;
		max = 100000000;
		step = 1;
		factor = 1;
	}
};

Array<AutoConfigData> get_auto_conf(PluginData *config)
{
	Script::SyntaxTree *ps = config->type->owner;
	Array<AutoConfigData> r;
	foreach(Script::ClassElement &e, config->type->element)
		if (e.type == Script::TypeFloat){
			AutoConfigData a;
			a.name = e.name;
			a.value = (float*)((char*)config + e.offset);
			foreach(Script::Constant &c, ps->Constants){
				if (c.name == "AutoConfig" + e.name){
					Array<string> p = c.value.explode(":");
					if (p.num == 5){
						a.min = p[0]._float();
						a.max = p[1]._float();
						a.step = p[2]._float();
						a.factor = p[3]._float();
						a.unit = p[4];
					}
				}
			}
			r.add(a);
		}
	return r;
}

class AutoConfigPanel : public HuiPanel
{
public:
	Array<Slider*> slider;
	Array<AutoConfigData> aa;
	AutoConfigPanel(Array<AutoConfigData> &_aa)
	{
		aa = _aa;
		AddControlTable("", 0, 0, 1, 3, "root-table");
		SetTarget("root-table", 0);
		AddControlTable("", 0, 1, 4, aa.num, "main-table");
		SetTarget("main-table", 0);
		foreachi(AutoConfigData &a, aa, i){
			AddText(a.name, 0, i, 0, 0, "");
			AddSlider("!width=150", 1, i, 0, 0, "slider-" + i);
			AddSpinButton(format("%f\\%f\\%f\\%f", *a.value, a.min*a.factor, a.max*a.factor, a.step), 2, i, 0, 0, "spin-" + i);
			AddText(a.unit, 3, i, 0, 0, "");
			slider.add(new Slider(this, "slider-" + i, "spin-" + i, a.min, a.max, a.factor, (void(HuiEventHandler::*)())&AutoConfigPanel::OnChange, *a.value, this));
		}
		tsunami->plugin_manager->PutCommandBarSizable(this, "root-table", 0, 2);
		tsunami->plugin_manager->PutFavoriteBarSizable(this, "root-table", 0, 0);
	}
	~AutoConfigPanel()
	{
		foreach(Slider *s, slider)
			delete(s);
	}
	void OnChange()
	{
		foreachi(AutoConfigData &a, aa, i){
			*a.value = slider[i]->Get();
		}
	}
};

// default handler...
void Configurable::Configure()
{
	PluginData *config = get_config();
	if (!config){
		tsunami->log->Warning(_("nichts zu konfigurieren"));
		return;
	}

	HuiPanel *panel = CreatePanel();
	if (!panel){
		Array<AutoConfigData> aa = get_auto_conf(config);
		panel = new AutoConfigPanel(aa);
	}
	HuiDialog *dlg = new HuiDialog(name, 300, 100, tsunami, false);
	dlg->AddControlTable("", 0, 0, 1, 3, "root-table");
	dlg->SetTarget("root-table", 0);
	tsunami->plugin_manager->PutCommandBarSizable(dlg, "root-table", 0, 2);
	tsunami->plugin_manager->PutFavoriteBarSizable(dlg, "root-table", 0, 0);
	dlg->Embed(panel, "root-table", 0, 1);
	dlg->Run();
}

void Configurable::notify()
{
	Notify("Change");
}



