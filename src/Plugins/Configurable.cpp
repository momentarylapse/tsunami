/*
 * Configurable.cpp
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */


#include "Configurable.h"

#include "../Audio/Source/SongRenderer.h"
#include "ConfigPanel.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../lib/kaba/kaba.h"
#include "PluginManager.h"
#include "../Stuff/Log.h"
#include "../View/Helper/Slider.h"
#include "../View/Helper/Progress.h"
#include "../View/AudioView.h"
#include "../Audio/Synth/DummySynthesizer.h"
#include "../Device/OutputStream.h"

const string Configurable::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";


void PluginData::__init__()
{
	new(this) PluginData;
}

void PluginData::__delete__()
{
	this->PluginData::~PluginData();
}


Array<Kaba::ClassElement> get_unique_elements(Kaba::Class *c)
{
	Array<Kaba::ClassElement> r;
	for (auto &e: c->elements)
		if (!e.hidden)
			r.add(e);
	return r;
}

string var_to_string(Kaba::Class *c, char *v)
{
	string r;
	if (c == Kaba::TypeInt){
		r += i2s(*(int*)v);
	}else if (c == Kaba::TypeChar){
		r += i2s(*(char*)v);
	}else if (c == Kaba::TypeFloat32){
		r += f2s(*(float*)v, 6);
	}else if (c == Kaba::TypeBool){
		r += (*(bool*)v) ? "true" : "false";
	}else if (c == Kaba::TypeString){
		r += "\"" + str_escape(*(string*)v) + "\"";
	}else if (c->is_array){
		r += "[";
		for (int i=0; i<c->array_length; i++){
			if (i > 0)
				r += " ";
			r += var_to_string(c->parent, &v[i * c->parent->size]);
		}
		r += "]";
	}else if (c->is_super_array){
		DynamicArray *a = (DynamicArray*)v;
		r += "[";
		for (int i=0; i<a->num; i++){
			if (i > 0)
				r += " ";
			r += var_to_string(c->parent, &(((char*)a->data)[i * c->parent->size]));
		}
		r += "]";
	}else if (c->name == "SampleRef*"){
		SampleRef *sr = *(SampleRef**)v;
		if (sr)
			r += i2s(sr->origin->get_index());
		else
			r += "nil";
	}else{
		Array<Kaba::ClassElement> e = get_unique_elements(c);
		r += "(";
		for (int i=0; i<e.num; i++){
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
		if ((i == start) and (var_temp[i] == '"')){
			in_string = true;
		}else if (in_string){
			if (var_temp[i] == '\\'){
				i ++;
			}else if (var_temp[i] == '"'){
				pos = i + 1;
				return str_unescape(var_temp.substr(start + 1, i - start - 1));
			}
		}else if ((var_temp[i] == ' ') or (var_temp[i] == ']') or (var_temp[i] == ')') or (var_temp[i] == '[') or (var_temp[i] == '(')){
			pos = i;
			return var_temp.substr(start, i - start);
		}
	}
	return var_temp.substr(start, -1);
}

void var_from_string(Kaba::Class *type, char *v, const string &s, int &pos, Song *song)
{
	if (pos >= s.num)
		return;
	if (type == Kaba::TypeInt){
		*(int*)v = get_next(s, pos)._int();
	}else if (type == Kaba::TypeChar){
		*(char*)v = get_next(s, pos)._int();
	}else if (type == Kaba::TypeFloat32){
		*(float*)v = get_next(s, pos)._float();
	}else if (type == Kaba::TypeBool){
		*(bool*)v = (get_next(s, pos) == "true");
	}else if (type == Kaba::TypeString){
		*(string*)v = get_next(s, pos);
	}else if (type->is_array){
		pos ++; // '['
		for (int i=0;i<type->array_length;i++){
			if (i > 0)
				pos ++; // ' '
			var_from_string(type->parent, &v[i * type->parent->size], s, pos, song);
		}
		pos ++; // ']'
	}else if (type->is_super_array){
		pos ++; // '['
		DynamicArray *a = (DynamicArray*)v;
		a->clear(); // todo...
		while (true){
			if ((s[pos] == ']') or (pos >= s.num))
				break;
			if (a->num > 0)
				pos ++; // ' '
			a->resize(a->num + 1);
			var_from_string(type->parent, &(((char*)a->data)[(a->num - 1) * type->parent->size]), s, pos, song);
		}
		pos ++; // ']'
	}else if (type->name == "SampleRef*"){
		string ss = get_next(s, pos);
		*(SampleRef**)v = NULL;
		if ((ss != "nil") and song){
			int n = ss._int();
			if ((n >= 0) and (n < song->samples.num)){
				*(SampleRef**)v = new SampleRef(song->samples[n]);
			}
		}
	}else{
		Array<Kaba::ClassElement> e = get_unique_elements(type);
		pos ++; // '('
		for (int i=0; i<e.num; i++){
			if (i > 0)
				pos ++; // ' '
			var_from_string(e[i].type, &v[e[i].offset], s, pos, song);
		}
		pos ++; // ')'
	}
}

Configurable::Configurable(int type)
{
	configurable_type = type;
	song = NULL;
}

Configurable::~Configurable()
{
}

void Configurable::__init__()
{
	new(this) Configurable(-1);
}

void Configurable::__delete__()
{
	this->Configurable::~Configurable();
}

PluginData *Configurable::get_config() const
{
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c)
		return NULL;
	for (auto &e: c->elements)
		if ((e.name == "config") and (e.type->get_root()->name == "PluginData")){
			PluginData *config = (PluginData*)((char*)this + e.offset);
			config->_class = e.type;
			return config;
		}
	return NULL;
}

PluginData *Configurable::get_state() const
{
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c){
		const DummySynthesizer *ds = dynamic_cast<const DummySynthesizer*>(this);
		if (ds)
			return (PluginData*)&ds->state;
		return NULL;
	}
	for (auto &e: c->elements)
		if ((e.name == "state") and (e.type->get_root()->name == "PluginData")){
			PluginData *state = (PluginData*)((char*)this + e.offset);
			state->_class = e.type;
			return state;
		}
	return NULL;
}

string Configurable::configToString() const
{
	PluginData *config = get_config();
	if (!config)
		return "";

	string s = var_to_string(config->_class, (char*)config);
	return s;
}

void Configurable::configFromString(const string &param)
{
	PluginData *config = get_config();
	if (!config)
		return;

	config->reset();
	int pos = 0;
	var_from_string(config->_class, (char*)config, param, pos, song);
	onConfig();
}


// default version of ResetConfig()
//   try to execute   Configurable.config.reset()
void Configurable::resetConfig()
{
	PluginData *config = get_config();
	if (config)
		config->reset();
	onConfig();
}

// default version of ResetState()
//   try to execute   Configurable.state.reset()
void Configurable::resetState()
{
	PluginData *state = get_state();
	if (state)
		state->reset();
	onConfig();
}

string to_camel_case(const string &s)
{
	string r;
	bool next_upper = true;
	for (int i=0; i<s.num; i++){
		if (s[i] == '_'){
			next_upper = true;
			continue;
		}else{
			if (next_upper)
				r += s.substr(i, 1).upper();
			else
				r += s.substr(i, 1);
			next_upper = false;
		}
	}
	return r;
}

struct AutoConfigData
{
	enum{
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_STRING,
		TYPE_PITCH,
	};
	string name, label;
	int type;
	AutoConfigData(int _type, const string &_name)
	{
		type = _type;
		name = _name;
		label = to_camel_case(_name);
	}
	virtual ~AutoConfigData(){}

	bool name_match(const string &const_name)
	{
		return (("AUTO_CONFIG_" + name).upper().replace("_", "") == const_name.upper().replace("_", ""));
	}
	virtual void parse(const string &s) = 0;
	virtual void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) = 0;
	virtual void get_value() = 0;
	virtual void set_value() = 0;
};

struct AutoConfigDataFloat : public AutoConfigData
{
	string unit;
	float min, max, step, factor;
	float *value;
	Slider *slider;
	AutoConfigDataFloat(const string &_name) :
		AutoConfigData(TYPE_FLOAT, _name)
	{
		min = -100000000;
		max = 100000000;
		step = 1;
		factor = 1;
		value = NULL;
		slider = NULL;
	}
	virtual ~AutoConfigDataFloat()
	{
		delete(slider);
	}
	virtual void parse(const string &s)
	{
		Array<string> p = s.explode(":");
		if (p.num == 5){
			min = p[0]._float();
			max = p[1]._float();
			step = p[2]._float();
			factor = p[3]._float();
			unit = p[4];
		}else{
			msg_write("required format: min:max:step:factor:unit");
		}

	}
	virtual void add_gui(ConfigPanel *p, int i, const hui::Callback &callback)
	{
		p->addGrid("", 1, i, 3, 1, "grid-" + i);
		p->setTarget("grid-" + i, 0);
		p->addSlider("!width=150,expandx", 0, 0, 0, 0, "slider-" + i);
		p->addSpinButton(format("%f\\%f\\%f\\%f", *value, min*factor, max*factor, step), 1, 0, 0, 0, "spin-" + i);
		p->addLabel(unit, 2, 0, 0, 0, "");
		slider = new Slider(p, "slider-" + i, "spin-" + i, min, max, factor, callback, *value);
	}
	virtual void get_value()
	{
		*value = slider->get();
	}
	virtual void set_value()
	{
		slider->set(*value);
	}
};

struct AutoConfigDataInt : public AutoConfigData
{
	int min, max;
	int *value;
	ConfigPanel *panel;
	string id;
	AutoConfigDataInt(const string &_name) :
		AutoConfigData(TYPE_INT, _name)
	{
		min = 0;
		max = 1000;
		value = NULL;
		panel = NULL;
	}
	virtual ~AutoConfigDataInt()
	{}
	virtual void parse(const string &s)
	{
		Array<string> p = s.explode(":");
		if (p.num == 2){
			min = p[0]._int();
			max = p[1]._int();
		}else{
			msg_write("required format: min:max");
		}

	}
	virtual void add_gui(ConfigPanel *p, int i, const hui::Callback &callback)
	{
		id = "spin-" + i;
		panel = p;
		p->addSpinButton(format("!width=150,expandx\\%d\\%d\\%d", *value, min, max), 1, i, 0, 0, id);
		p->event(id, callback);
	}
	virtual void get_value()
	{
		*value = panel->getInt(id);
	}
	virtual void set_value()
	{
		panel->setInt(id, *value);
	}
};

struct AutoConfigDataPitch : public AutoConfigData
{
	float *value;
	string id;
	ConfigPanel *panel;
	AutoConfigDataPitch(const string &_name) :
		AutoConfigData(TYPE_PITCH, _name)
	{
		value = NULL;
		panel = NULL;
	}
	virtual ~AutoConfigDataPitch()
	{}
	virtual void parse(const string &s)
	{}
	virtual void add_gui(ConfigPanel *p, int i, const hui::Callback &callback)
	{
		id = "pitch-" + i;
		panel = p;
		p->addComboBox("!width=150,expandx", 1, i, 0, 0, id);
		for (int j=0; j<MAX_PITCH; j++)
			p->addString(id, pitch_name(j));
		p->setInt(id, *value);
		p->event(id, callback);
	}
	virtual void get_value()
	{
		*value = panel->getInt(id);
	}
	virtual void set_value()
	{
		panel->setInt(id, *value);
	}
};

struct AutoConfigDataString : public AutoConfigData
{
	string *value;
	string id;
	ConfigPanel *panel;
	AutoConfigDataString(const string &_name) :
		AutoConfigData(TYPE_STRING, _name)
	{
		value = NULL;
		panel = NULL;
	}
	virtual ~AutoConfigDataString()
	{
	}
	virtual void parse(const string &s)
	{
	}
	virtual void add_gui(ConfigPanel *p, int i, const hui::Callback &callback)
	{
		id = "edit-" + i;
		panel = p;
		p->addEdit("!width=150,expandx\\" + *value, 1, i, 0, 0, id);
		p->event(id, callback);
	}
	virtual void get_value()
	{
		*value = panel->getString(id);
	}
	virtual void set_value()
	{
		panel->setString(id, *value);
	}
};

Array<AutoConfigData*> get_auto_conf(PluginData *config)
{
	Kaba::SyntaxTree *ps = config->_class->owner;
	Array<AutoConfigData*> r;
	for (auto &e: config->_class->elements){
		if (e.type == Kaba::TypeFloat32){
			if (e.name == "pitch"){
				AutoConfigDataPitch *a = new AutoConfigDataPitch(e.name);
				a->value = (float*)((char*)config + e.offset);
				r.add(a);
			}else{
				AutoConfigDataFloat *a = new AutoConfigDataFloat(e.name);
				a->value = (float*)((char*)config + e.offset);
				r.add(a);
			}
		}else if (e.type == Kaba::TypeInt){
			AutoConfigDataInt *a = new AutoConfigDataInt(e.name);
			a->value = (int*)((char*)config + e.offset);
			r.add(a);
		}else if (e.type == Kaba::TypeString){
			AutoConfigDataString *a = new AutoConfigDataString(e.name);
			a->value = (string*)((char*)config + e.offset);
			r.add(a);
		}
	}

	for (auto a: r){
		for (auto c: ps->constants){
			if (c->type == Kaba::TypeString)
				if (a->name_match(c->name))
					a->parse(c->as_string());
		}
	}

	return r;
}

class AutoConfigPanel : public ConfigPanel
{
public:
	Array<AutoConfigData*> aa;
	AutoConfigPanel(Array<AutoConfigData*> &_aa, Configurable *_c) :
		ConfigPanel(_c)
	{
		aa = _aa;
		addGrid("", 0, 0, 1, 3, "root-table");
		setTarget("root-table", 0);
		addGrid("", 0, 1, 4, aa.num, "main-table");
		setTarget("main-table", 0);
		foreachi(AutoConfigData *a, aa, i){
			setTarget("main-table", 0);
			addLabel(a->label, 0, i, 0, 0, "");
			a->add_gui(this, i, std::bind(&AutoConfigPanel::onChange, this));
		}
	}
	~AutoConfigPanel()
	{
		for (auto a: aa)
			delete a;
	}
	void _cdecl onChange()
	{
		for (auto a: aa)
			a->get_value();
		notify();

	}
	virtual void _cdecl update()
	{
		for (auto a: aa)
			a->set_value();
	}
};

// default handler...
ConfigPanel *Configurable::createPanel()
{
	PluginData *config = get_config();
	if (!config)
		return NULL;
	auto aa = get_auto_conf(config);
	if (aa.num == 0)
		return NULL;
	return new AutoConfigPanel(aa, this);
}

/*void Configurable::updateDialog()
{
	if (_auto_panel_){
		_auto_panel_->update();
	}
}*/

class ConfigurationDialog : public hui::Window
{
public:
	ConfigurationDialog(Configurable *c, PluginData *pd, ConfigPanel *p) :
		hui::Window("configurable_dialog", tsunami->win)
	{
		config = c;
		panel = p;
		progress = NULL;
		ok = false;

		setTitle(config->name);
		embed(panel, "grid", 0, 1);

		if (c->configurable_type != c->TYPE_EFFECT)
			hideControl("preview", true);

		event("load_favorite", std::bind(&ConfigurationDialog::onLoad, this));
		event("save_favorite", std::bind(&ConfigurationDialog::onSave, this));
		event("ok", std::bind(&ConfigurationDialog::onOk, this));
		event("preview", std::bind(&ConfigurationDialog::onPreview, this));
		event("cancel", std::bind(&ConfigurationDialog::onClose, this));
		event("hui:close", std::bind(&ConfigurationDialog::onClose, this));
	}
	~ConfigurationDialog()
	{
	}
	void onOk()
	{
		ok = true;
		destroy();
	}
	void onClose()
	{
		destroy();
	}
	void onPreview()
	{
		previewStart();
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

	void previewStart()
	{
		/*if (progress)
			previewEnd();
		config->configToString();
		tsunami->win->view->renderer->preview_effect = (Effect*)config;


		progress = new ProgressCancelable(_("Preview"), win);
		progress->subscribe(this, std::bind(&ConfigurationDialog::onProgressCancel, this), progress->MESSAGE_CANCEL);

		tsunami->win->view->stream->subscribe(this, std::bind(&ConfigurationDialog::onUpdateStream, this));
		tsunami->win->view->renderer->prepare(tsunami->win->view->sel.range, false);
		tsunami->win->view->stream->play();*/
	}

	void onProgressCancel()
	{
		previewEnd();
	}

	void onUpdateStream()
	{
		/*if (progress){
			int pos = tsunami->win->view->stream->getPos(0); // TODO
			Range r = tsunami->win->view->sel.range;
			progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
			if (!tsunami->win->view->stream->isPlaying())
				previewEnd();
		}*/
	}

	void previewEnd()
	{
		/*if (!progress)
			return;
		tsunami->win->view->stream->unsubscribe(this);
		progress->unsubscribe(this);
		tsunami->win->view->stream->stop();
		delete(progress);
		progress = NULL;


		tsunami->win->view->renderer->preview_effect = NULL;*/
	}

	Configurable *config;
	ConfigPanel *panel;
	Progress *progress;
	bool ok;
};

bool Configurable::configure()
{
	PluginData *config = get_config();
	if (!config)
		return true;

	//_auto_panel_ = NULL;
	ConfigPanel *panel = createPanel();
	if (!panel)
		return true;
	ConfigurationDialog *dlg = new ConfigurationDialog(this, config, panel);
	dlg->run();
	bool ok = dlg->ok;
	delete(dlg);
	return ok;
}

void Configurable::notify()
{
	onConfig();
	Observable::notify();
}

Configurable *Configurable::copy() const
{
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c){
		if (this->configurable_type == TYPE_SYNTHESIZER)
			return new DummySynthesizer;
		return NULL;
	}

	Configurable *clone = (Configurable*)c->create_instance();

	clone->configurable_type = configurable_type;
	clone->configFromString(configToString());

	return clone;
}



