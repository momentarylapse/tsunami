/*
 * Configurable.cpp
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */


#include "Configurable.h"
#include "ConfigPanel.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../lib/script/script.h"
#include "PluginManager.h"
#include "../Stuff/Log.h"
#include "../View/Helper/Slider.h"

void GlobalRemoveSliders(HuiPanel*);

const string Configurable::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";


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
	foreach(Script::ClassElement &e, t->element)
		if (!e.hidden){
			r.add(e);
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
	}else if (type == Script::TypeFloat32){
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
	}else if (type == Script::TypeFloat32){
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

Configurable::Configurable(const string &observable_name, int type) :
	Observable(observable_name)
{
	configurable_type = type;
}

Configurable::~Configurable()
{
}

void Configurable::__init__()
{
	new(this) Configurable("", -1);
}

void Configurable::__delete__()
{
	this->Observable::~Observable();
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
		value = NULL;
	}
};

Array<AutoConfigData> get_auto_conf(PluginData *config)
{
	Script::SyntaxTree *ps = config->type->owner;
	Array<AutoConfigData> r;
	foreach(Script::ClassElement &e, config->type->element)
		if (e.type == Script::TypeFloat32){
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

class AutoConfigPanel : public ConfigPanel
{
public:
	Array<Slider*> slider;
	Array<AutoConfigData> aa;
	AutoConfigPanel(Array<AutoConfigData> &_aa, Configurable *_c) :
		ConfigPanel(_c)
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
			slider.add(new Slider(this, "slider-" + i, "spin-" + i, a.min, a.max, a.factor, (void(HuiEventHandler::*)())&AutoConfigPanel::onChange, *a.value, this));
		}
	}
	~AutoConfigPanel()
	{
		foreach(Slider *s, slider)
			delete(s);
	}
	void onChange()
	{
		foreachi(AutoConfigData &a, aa, i){
			*a.value = slider[i]->Get();
		}
		notify();

	}
	virtual void update()
	{
		foreachi(AutoConfigData &a, aa, i){
			slider[i]->Set(*a.value);
		}
	}
};

// default handler...
ConfigPanel *Configurable::CreatePanel()
{
	PluginData *config = get_config();
	if (!config)
		return NULL;
	Array<AutoConfigData> aa = get_auto_conf(config);
	if (aa.num == 0)
		return NULL;
	return new AutoConfigPanel(aa, this);
}

/*void Configurable::UpdateDialog()
{
	if (_auto_panel_){
		_auto_panel_->update();
	}
}*/

class ConfigurationDialog : public HuiDialog
{
public:
	ConfigurationDialog(Configurable *c, PluginData *pd, ConfigPanel *p) :
		HuiDialog(c->name, 300, 100, tsunami->win, false)
	{
		config = c;
		panel = p;
		AddControlTable("", 0, 0, 1, 3, "root-table");
		SetTarget("root-table", 0);
		Embed(panel, "root-table", 0, 1);

		// favorite grid
		SetTarget("root-table", 0);
		AddControlTable("!noexpandy", 0, 0, 5, 1, "favorite_grid");
		SetTarget("favorite_grid", 0);
		AddButton("!flat", 0, 0, 0, 0, "load_favorite");
		SetImage("load_favorite", "hui:open");
		SetTooltip("load_favorite", _("Parameter laden"));
		AddButton("!flat", 1, 0, 0, 0, "save_favorite");
		SetImage("save_favorite", "hui:save");
		SetTooltip("save_favorite", _("Parameter speichern"));
		EventM("load_favorite", this, &ConfigurationDialog::onLoad);
		EventM("save_favorite", this, &ConfigurationDialog::onSave);

		// command grid
		SetTarget("root-table", 0);
		AddControlTable("!buttonbar", 0, 2, 4, 1, "command_grid");
		SetTarget("command_grid", 0);
		if (c->configurable_type == c->TYPE_EFFECT){
			AddButton(_("Vorschau"), 0, 0, 0, 0, "preview");
			SetImage("preview", "hui:media-play");
		}
		AddText("!width=30", 1, 0, 0, 0, "");
		AddButton(_("Abbrechen"), 2, 0, 0, 0, "cancel");
		SetImage("cancel", "hui:cancel");
		AddDefButton(_("OK"), 3, 0, 0, 0, "ok");
		SetImage("ok", "hui:ok");
		EventM("ok", this, &ConfigurationDialog::onOk);
		EventM("preview", this, &ConfigurationDialog::onPreview);
		EventM("cancel", this, &ConfigurationDialog::onClose);
		EventM("hui:close", this, &ConfigurationDialog::onClose);
	}
	~ConfigurationDialog()
	{
		GlobalRemoveSliders(this);
	}
	void onOk()
	{
		delete(this);
	}
	void onClose()
	{
		delete(this);
	}
	void onPreview()
	{
		tsunami->plugin_manager->PreviewStart((Effect*)config);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(this, config, false);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->ApplyFavorite(config, name);
		panel->update();
	}
	void onSave()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(this, config, true);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->SaveFavorite(config, name);
	}

	Configurable *config;
	ConfigPanel *panel;
};

bool Configurable::Configure()
{
	PluginData *config = get_config();
	if (!config)
		return true;

	//_auto_panel_ = NULL;
	ConfigPanel *panel = CreatePanel();
	if (!panel)
		return false;
	HuiDialog *dlg = new ConfigurationDialog(this, config, panel);
	return (dlg->Run() == "ok");
}

void Configurable::notify()
{
	Notify();
}



