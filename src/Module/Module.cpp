/*
 * Module.cpp
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */


#include "../Plugins/ConfigPanel.h"
#include "../Plugins/AutoConfigPanel.h"
#include "../Session.h"
#include "../lib/kaba/kaba.h"
#include "../Plugins/PluginManager.h"
#include "../Plugins/Plugin.h"
#include "../View/Helper/Progress.h"
#include "../View/AudioView.h"
#include "../Audio/Synth/DummySynthesizer.h"
#include "../Device/OutputStream.h"
#include "Module.h"

const string Module::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";


void ModuleConfiguration::__init__()
{
	new(this) ModuleConfiguration;
}

void ModuleConfiguration::__delete__()
{
	this->ModuleConfiguration::~ModuleConfiguration();
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

Module::Module(Session *_session, int type)
{
	module_type = type;
	session = _session;
	usable = true;
	plugin = NULL;
	enabled = true;
	module_x = module_y = 0;
}

Module::~Module()
{
}

void Module::__init__()
{
	new(this) Module(Session::GLOBAL, -1);
}

void Module::__delete__()
{
	this->Module::~Module();
}

ModuleConfiguration *Module::get_config() const
{
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c)
		return NULL;
	for (auto &e: c->elements)
		if ((e.name == "config") and (e.type->get_root()->name == "PluginData")){
			ModuleConfiguration *config = (ModuleConfiguration*)((char*)this + e.offset);
			config->_class = e.type;
			return config;
		}
	return NULL;
}

string Module::config_to_string() const
{
	ModuleConfiguration *config = get_config();
	if (!config)
		return "";

	string s = var_to_string(config->_class, (char*)config);
	return s;
}

void Module::config_from_string(const string &param)
{
	ModuleConfiguration *config = get_config();
	if (!config)
		return;

	config->reset();
	int pos = 0;
	var_from_string(config->_class, (char*)config, param, pos, session->song);
	on_config();
}


// default version of ResetConfig()
//   try to execute   Module.config.reset()
void Module::reset_config()
{
	ModuleConfiguration *config = get_config();
	if (config)
		config->reset();
	on_config();
}

// default handler...
ConfigPanel *Module::create_panel()
{
	ModuleConfiguration *config = get_config();
	if (!config)
		return NULL;
	auto aa = get_auto_conf(config);
	if (aa.num == 0)
		return NULL;
	return new AutoConfigPanel(aa, this);
}

string Module::get_error()
{
	if (plugin)
		return plugin->get_error();
	return format(_("Can't load %s: \"%s\""), type_to_name(module_type).c_str(), name.c_str());
}


/*void Module::updateDialog()
{
	if (_auto_panel_){
		_auto_panel_->update();
	}
}*/

class ConfigurationDialog : public hui::Window
{
public:
	ConfigurationDialog(Module *c, ModuleConfiguration *pd, ConfigPanel *p, hui::Window *parent) :
		hui::Window("configurable_dialog", parent)
	{
		config = c;
		panel = p;
		progress = NULL;
		ok = false;

		setTitle(config->name);
		embed(panel, "grid", 0, 1);

		if (c->module_type != c->Type::AUDIO_EFFECT)
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

	Module *config;
	ConfigPanel *panel;
	Progress *progress;
	bool ok;
};

bool Module::configure(hui::Window *win)
{
	ModuleConfiguration *config = get_config();
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

void Module::notify()
{
	on_config();
	Observable::notify();
}

Module *Module::copy() const
{
	Kaba::Class *c = Kaba::GetDynamicType(this);
	if (!c){
		if (this->module_type == Type::SYNTHESIZER)
			return new DummySynthesizer;
		return NULL;
	}
	Module *clone = (Module*)c->create_instance();

	clone->module_type = module_type;
	clone->session = session;
	clone->config_from_string(config_to_string());

	return clone;
}


string Module::type_to_name(int type)
{
	if (type == Module::Type::AUDIO_SOURCE)
		return "AudioSource";
	if (type == Module::Type::AUDIO_EFFECT)
		return "AudioEffect";
	if (type == Module::Type::SYNTHESIZER)
		return "Synthesizer";
	if (type == Module::Type::MIDI_SOURCE)
		return "MidiSource";
	if (type == Module::Type::MIDI_EFFECT)
		return "MidiEffect";
	if (type == Module::Type::BEAT_SOURCE)
		return "BeatSource";
	if (type == Module::Type::PEAK_METER)
		return "PeakMeter";
	if (type == Module::Type::PITCH_DETECTOR)
		return "PitchDetector";
	if (type == Module::Type::OUTPUT_STREAM_AUDIO)
		return "OutputStream";
	if (type == Module::Type::INPUT_STREAM_AUDIO)
		return "InputStreamAudio";
	if (type == Module::Type::INPUT_STREAM_MIDI)
		return "InputStreamMidi";
	if (type == Module::Type::AUDIO_SUCKER)
		return "AudioSucker";
	if (type == Module::Type::AUDIO_JOINER)
		return "AudioJoiner";
	if (type == Module::Type::BEAT_MIDIFIER)
		return "BeatMidifier";
	return "???";
}

Module::Type Module::type_from_name(const string &str)
{
	if (str == "AudioSource")
		return Module::Type::AUDIO_SOURCE;
	if (str == "AudioEffect" or str == "Effect")
		return Module::Type::AUDIO_EFFECT;
	if (str == "Synthesizer" or str == "Synth")
		return Module::Type::SYNTHESIZER;
	if (str == "MidiEffect")
		return Module::Type::MIDI_EFFECT;
	if (str == "MidiSource")
		return Module::Type::MIDI_SOURCE;
	if (str == "BeatSource")
		return Module::Type::BEAT_SOURCE;
	return (Type)-1;
}


