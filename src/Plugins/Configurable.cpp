/*
 * Configurable.cpp
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */


#include "Configurable.h"

#include "ConfigPanel.h"
#include "AutoConfigPanel.h"
#include "../Session.h"
#include "../lib/kaba/kaba.h"
#include "PluginManager.h"
#include "Plugin.h"
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

Configurable::Configurable(Session *_session, int type)
{
	configurable_type = type;
	session = _session;
	song = NULL;
	usable = true;
	plugin = NULL;
	enabled = true;
}

Configurable::~Configurable()
{
}

void Configurable::__init__()
{
	new(this) Configurable(Session::GLOBAL, -1);
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

string Configurable::config_to_string() const
{
	PluginData *config = get_config();
	if (!config)
		return "";

	string s = var_to_string(config->_class, (char*)config);
	return s;
}

void Configurable::config_from_string(const string &param)
{
	PluginData *config = get_config();
	if (!config)
		return;

	config->reset();
	int pos = 0;
	var_from_string(config->_class, (char*)config, param, pos, session->song);
	on_config();
}


// default version of ResetConfig()
//   try to execute   Configurable.config.reset()
void Configurable::reset_config()
{
	PluginData *config = get_config();
	if (config)
		config->reset();
	on_config();
}

// default handler...
ConfigPanel *Configurable::create_panel()
{
	PluginData *config = get_config();
	if (!config)
		return NULL;
	auto aa = get_auto_conf(config);
	if (aa.num == 0)
		return NULL;
	return new AutoConfigPanel(aa, this);
}

string Configurable::get_error()
{
	if (plugin)
		return plugin->get_error();
	return format(_("Can't load %s: \"%s\""), type_to_name(configurable_type).c_str(), name.c_str());
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
	ConfigurationDialog(Configurable *c, PluginData *pd, ConfigPanel *p, hui::Window *parent) :
		hui::Window("configurable_dialog", parent)
	{
		config = c;
		panel = p;
		progress = NULL;
		ok = false;

		setTitle(config->name);
		embed(panel, "grid", 0, 1);

		if (c->configurable_type != c->Type::AUDIO_EFFECT)
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
		string name = config->session->plugin_manager->SelectFavoriteName(this, config, false);
		if (name.num == 0)
			return;
		config->session->plugin_manager->ApplyFavorite(config, name);
		panel->update();
	}
	void onSave()
	{
		string name = config->session->plugin_manager->SelectFavoriteName(this, config, true);
		if (name.num == 0)
			return;
		config->session->plugin_manager->SaveFavorite(config, name);
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

bool Configurable::configure(hui::Window *win)
{
	PluginData *config = get_config();
	if (!config)
		return true;

	//_auto_panel_ = NULL;
	ConfigPanel *panel = create_panel();
	if (!panel)
		return true;
	ConfigurationDialog *dlg = new ConfigurationDialog(this, config, panel, win);
	dlg->run();
	bool ok = dlg->ok;
	delete(dlg);
	return ok;
}

void Configurable::notify()
{
	on_config();
	Observable::notify();
}

Configurable *Configurable::copy() const
{
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c){
		if (this->configurable_type == Type::SYNTHESIZER)
			return new DummySynthesizer;
		return NULL;
	}
	Configurable *clone = (Configurable*)c->create_instance();

	clone->configurable_type = configurable_type;
	clone->session = session;
	clone->song = song;
	clone->config_from_string(config_to_string());

	return clone;
}


string Configurable::type_to_name(int type)
{
	if (type == Configurable::Type::AUDIO_SOURCE)
		return "AudioSource";
	if (type == Configurable::Type::AUDIO_EFFECT)
		return "AudioEffect";
	if (type == Configurable::Type::SYNTHESIZER)
		return "Synthesizer";
	if (type == Configurable::Type::MIDI_SOURCE)
		return "MidiSource";
	if (type == Configurable::Type::MIDI_EFFECT)
		return "MidiEffect";
	if (type == Configurable::Type::BEAT_SOURCE)
		return "BeatSource";
	return "???";
}

Configurable::Type Configurable::type_from_name(const string &str)
{
	if (str == "AudioSource")
		return Configurable::Type::AUDIO_SOURCE;
	if (str == "AudioEffect" or str == "Effect")
		return Configurable::Type::AUDIO_EFFECT;
	if (str == "Synthesizer" or str == "Synth")
		return Configurable::Type::SYNTHESIZER;
	if (str == "MidiEffect")
		return Configurable::Type::MIDI_EFFECT;
	if (str == "MidiSource")
		return Configurable::Type::MIDI_SOURCE;
	if (str == "BeatSource")
		return Configurable::Type::BEAT_SOURCE;
	return (Type)-1;
}


