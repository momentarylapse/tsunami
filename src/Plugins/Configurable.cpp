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
#include "../lib/kaba/kaba.h"
#include "PluginManager.h"
#include "../Stuff/Log.h"
#include "../View/Helper/Slider.h"
#include "../View/Helper/Progress.h"
#include "../View/AudioView.h"
#include "../Audio/Synth/DummySynthesizer.h"
#include "../Audio/Renderer/SongRenderer.h"
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

Configurable::Configurable(const string &observable_name, int type) :
	Observable(observable_name)
{
	configurable_type = type;
	song = NULL;
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
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c)
		return NULL;
	for (auto &e: c->elements)
		if ((e.name == "config") and (e.type->GetRoot()->name == "PluginData")){
			PluginData *config = (PluginData*)((char*)this + e.offset);
			config->_class = e.type;
			return config;
		}
	return NULL;
}

PluginData *Configurable::get_state()
{
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c){
		DummySynthesizer *ds = dynamic_cast<DummySynthesizer*>(this);
		if (ds)
			return &ds->state;
		return NULL;
	}
	for (auto &e: c->elements)
		if ((e.name == "state") and (e.type->GetRoot()->name == "PluginData")){
			PluginData *state = (PluginData*)((char*)this + e.offset);
			state->_class = e.type;
			return state;
		}
	return NULL;
}

string Configurable::configToString()
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

struct AutoConfigData
{
	string name, unit;
	float min, max, step, factor;
	float *value;
	Slider *slider;
	AutoConfigData()
	{
		min = -100000000;
		max = 100000000;
		step = 1;
		factor = 1;
		value = NULL;
		slider = NULL;
	}
};

Array<AutoConfigData> get_auto_conf(PluginData *config)
{
	Kaba::SyntaxTree *ps = config->_class->owner;
	Array<AutoConfigData> r;
	for (auto &e: config->_class->elements)
		if (e.type == Kaba::TypeFloat32){
			AutoConfigData a;
			a.name = e.name;
			a.value = (float*)((char*)config + e.offset);
			for (auto &c: ps->constants){
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
	Array<AutoConfigData> aa;
	AutoConfigPanel(Array<AutoConfigData> &_aa, Configurable *_c) :
		ConfigPanel(_c)
	{
		aa = _aa;
		addGrid("", 0, 0, 1, 3, "root-table");
		setTarget("root-table", 0);
		addGrid("", 0, 1, 4, aa.num, "main-table");
		setTarget("main-table", 0);
		foreachi(AutoConfigData &a, aa, i){
			addLabel(a.name, 0, i, 0, 0, "");
			addSlider("!width=150", 1, i, 0, 0, "slider-" + i);
			addSpinButton(format("%f\\%f\\%f\\%f", *a.value, a.min*a.factor, a.max*a.factor, a.step), 2, i, 0, 0, "spin-" + i);
			addLabel(a.unit, 3, i, 0, 0, "");
			a.slider = new Slider(this, "slider-" + i, "spin-" + i, a.min, a.max, a.factor, (void(HuiEventHandler::*)())&AutoConfigPanel::onChange, *a.value, this);
		}
	}
	~AutoConfigPanel()
	{
		for (auto &a: aa)
			delete(a.slider);
	}
	void _cdecl onChange()
	{
		for (auto &a: aa)
			*a.value = a.slider->get();
		notify();

	}
	virtual void _cdecl update()
	{
		for (auto &a: aa)
			a.slider->set(*a.value);
	}
};

// default handler...
ConfigPanel *Configurable::createPanel()
{
	PluginData *config = get_config();
	if (!config)
		return NULL;
	Array<AutoConfigData> aa = get_auto_conf(config);
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

class ConfigurationDialog : public HuiWindow, public Observer
{
public:
	ConfigurationDialog(Configurable *c, PluginData *pd, ConfigPanel *p) :
		HuiWindow("configurable_dialog", tsunami->win),
		Observer("")
	{
		config = c;
		panel = p;
		progress = NULL;
		ok = false;

		setTitle(config->name);
		embed(panel, "grid", 0, 1);

		if (c->configurable_type != c->TYPE_EFFECT)
			hideControl("preview", true);

		event("load_favorite", this, &ConfigurationDialog::onLoad);
		event("save_favorite", this, &ConfigurationDialog::onSave);
		event("ok", this, &ConfigurationDialog::onOk);
		event("preview", this, &ConfigurationDialog::onPreview);
		event("cancel", this, &ConfigurationDialog::onClose);
		event("hui:close", this, &ConfigurationDialog::onClose);
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
		if (progress)
			previewEnd();
		config->configToString();
		tsunami->win->view->renderer->effect = (Effect*)config;


		progress = new ProgressCancelable(_("Preview"), win);
		subscribe(progress);
		subscribe(tsunami->win->view->stream);
		tsunami->win->view->renderer->prepare(tsunami->win->view->sel.range, false);
		tsunami->win->view->stream->play();
	}

	virtual void onUpdate(Observable *o, const string &message)
	{
		if (progress and (o == progress)){
			if (message == progress->MESSAGE_CANCEL)
				previewEnd();
		}else if (progress and (o == tsunami->win->view->stream)){
			int pos = tsunami->win->view->stream->getPos();
			Range r = tsunami->win->view->sel.range;
			progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
			if (!tsunami->win->view->stream->isPlaying())
				previewEnd();
		}
	}

	void previewEnd()
	{
		if (!progress)
			return;
		unsubscribe(tsunami->win->view->stream);
		unsubscribe(progress);
		tsunami->win->view->stream->stop();
		delete(progress);
		progress = NULL;


		tsunami->win->view->renderer->effect = NULL;
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
		return false;
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



