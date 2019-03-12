/*
 * AutoConfigPanel.cpp
 *
 *  Created on: 08.10.2017
 *      Author: michi
 */


#include "AutoConfigPanel.h"
#include "Module.h"
#include "ModuleConfiguration.h"
#include "../View/Helper/Slider.h"
#include "../View/SideBar/SampleManagerConsole.h"
#include "../Data/Song.h"
#include "../Data/Sample.h"
#include "../Data/SampleRef.h"
#include "../Data/Midi/MidiData.h"
#include "../lib/kaba/kaba.h"
#include "../Session.h"
#include "../Device/Device.h"
#include "../Device/DeviceManager.h"


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
	enum class Type{
		FLOAT,
		INT,
		BOOL,
		STRING,
		PITCH,
		SAMPLE_REF,
		DEVICE,
	};
	string name, label, unit;
	Type type;
	AutoConfigData(Type _type, const string &_name)
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
	float min, max, step, factor;
	float *value;
	Slider *slider;
	AutoConfigDataFloat(const string &_name) :
		AutoConfigData(Type::FLOAT, _name)
	{
		min = -100000000;
		max = 100000000;
		step = 1;
		factor = 1;
		value = nullptr;
		slider = nullptr;
	}
	~AutoConfigDataFloat() override
	{
		delete(slider);
	}
	void parse(const string &s) override
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
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override
	{
		p->add_grid("", 1, i, "grid-" + i2s(i));
		p->set_target("grid-" + i2s(i));
		p->add_slider("!width=150,expandx", 0, 0, "slider-" + i2s(i));
		p->add_spin_button(f2s(*value, 6), 1, 0, "spin-" + i2s(i));
		p->set_options("spin-" + i2s(i), format("range=%f:%f:%f", min*factor, max*factor, step));
		//p->addLabel(unit, 2, 0, "");
		slider = new Slider(p, "slider-" + i2s(i), "spin-" + i2s(i), min, max, factor, callback, *value);
	}
	void get_value() override
	{
		*value = slider->get();
	}
	void set_value() override
	{
		slider->set(*value);
	}
};

struct AutoConfigDataBool : public AutoConfigData
{
	bool *value;
	ConfigPanel *panel;
	string id;
	AutoConfigDataBool(const string &_name) :
		AutoConfigData(Type::BOOL, _name)
	{
		value = nullptr;
		panel = nullptr;
	}
	~AutoConfigDataBool() override {};
	void parse(const string &s) override {};
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override
	{
		id = "check-" + i2s(i);
		panel = p;
		p->add_check_box("!width=150,expandx", 1, i, id);
		p->event(id, callback);
	}
	void get_value() override
	{
		*value = panel->is_checked(id);
	}
	void set_value() override
	{
		panel->check(id, *value);
	}
};

struct AutoConfigDataInt : public AutoConfigData
{
	int min, max;
	int *value;
	ConfigPanel *panel;
	string id;
	AutoConfigDataInt(const string &_name) :
		AutoConfigData(Type::INT, _name)
	{
		min = 0;
		max = 1000000;
		value = nullptr;
		panel = nullptr;
	}
	void parse(const string &s) override
	{
		Array<string> p = s.explode(":");
		if (p.num == 2){
			min = p[0]._int();
			max = p[1]._int();
		}else{
			msg_write("required format: min:max");
		}
	}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override
	{
		id = "spin-" + i2s(i);
		panel = p;
		p->add_spin_button("!width=150,expandx\\" + i2s(*value), 1, i, id);
		p->set_options(id, format("range=%d:%d", min, max));
		p->event(id, callback);
	}
	void get_value() override
	{
		*value = panel->get_int(id);
	}
	void set_value() override
	{
		panel->set_int(id, *value);
	}
};

struct AutoConfigDataPitch : public AutoConfigData
{
	float *value;
	string id;
	ConfigPanel *panel;
	AutoConfigDataPitch(const string &_name) :
		AutoConfigData(Type::PITCH, _name)
	{
		value = nullptr;
		panel = nullptr;
	}
	void parse(const string &s) override
	{}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override
	{
		id = "pitch-" + i2s(i);
		panel = p;
		p->add_combo_box("!width=150,expandx", 1, i, id);
		for (int j=0; j<MAX_PITCH; j++)
			p->add_string(id, pitch_name(j));
		p->set_int(id, *value);
		p->event(id, callback);
	}
	void get_value() override
	{
		*value = panel->get_int(id);
	}
	void set_value() override
	{
		panel->set_int(id, *value);
	}
};

struct AutoConfigDataString : public AutoConfigData
{
	string *value;
	string id;
	ConfigPanel *panel;
	AutoConfigDataString(const string &_name) :
		AutoConfigData(Type::STRING, _name)
	{
		value = nullptr;
		panel = nullptr;
	}
	void parse(const string &s) override
	{}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override
	{
		id = "edit-" + i2s(i);
		panel = p;
		p->add_edit("!width=150,expandx\\" + *value, 1, i, id);
		p->event(id, callback);
	}
	void get_value() override
	{
		*value = panel->get_string(id);
	}
	void set_value() override
	{
		panel->set_string(id, *value);
	}
};

struct AutoConfigDataSampleRef : public AutoConfigData
{
	SampleRef **value;
	string id;
	ConfigPanel *panel;
	Session *session;
	hui::Callback callback;
	AutoConfigDataSampleRef(const string &_name, Session *_session) :
		AutoConfigData(Type::SAMPLE_REF, _name)
	{
		value = nullptr;
		panel = nullptr;
		session = _session;
	}
	void parse(const string &s) override
	{}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &_callback) override
	{
		id = "sample-" + i2s(i);
		panel = p;
		p->add_button("!expandx", 1, i, id);
		set_value();
		p->event(id, [=]{ on_button(); });
		callback = _callback;
	}
	void on_button()
	{
		Sample *old = nullptr;
		if (*value)
			old = (*value)->origin;
		Sample *s = SampleManagerConsole::select(session, panel, old);
		if (s != old){
			if (*value)
				delete *value;
			*value = nullptr;
			if (s)
				*value = s->create_ref();
			set_value();
			callback();
		}
	}
	void get_value() override
	{
	}
	void set_value() override
	{
		if (*value)
			panel->set_string(id, (*value)->origin->name);
		else
			panel->set_string(id, _(" - none -"));
	}
};

struct AutoConfigDataDevice: public AutoConfigData
{
	Device **value;
	string id;
	ConfigPanel *panel;
	DeviceType type;
	Session *session;
	Array<Device*> list;
	AutoConfigDataDevice(const string &_name, Session *_session) :
		AutoConfigData(Type::DEVICE, _name)
	{
		session = _session;
		value = nullptr;
		panel = nullptr;
		type = DeviceType::AUDIO_OUTPUT;
	}
	void parse(const string &s) override
	{
		type = DeviceType::AUDIO_OUTPUT;
		if (s == "audio:input")
			type = DeviceType::AUDIO_INPUT;
		if (s == "midi:input")
			type = DeviceType::MIDI_INPUT;
		update_list();
	}
	void update_list()
	{
		list = session->device_manager->device_list(type);
		if (panel){
			panel->reset(id);
			for (auto *d: list)
				panel->add_string(id, d->name);
			panel->set_int(id, list.find(*value));
		}
	}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override
	{
		id = "device-" + i2s(i);
		panel = p;
		p->add_combo_box("!width=150,expandx", 1, i, id);
		p->event(id, callback);
		update_list();
	}
	void get_value() override
	{
		*value = session->device_manager->choose_device(type);
		int n = panel->get_int(id);
		if (n >= 0)
			*value = list[n];
	}
	void set_value() override
	{
		panel->set_int(id, list.find(*value));
	}
};

Array<AutoConfigData*> get_auto_conf(ModuleConfiguration *config, Session *session)
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
		}else if (e.type == Kaba::TypeBool){
			AutoConfigDataBool *a = new AutoConfigDataBool(e.name);
			a->value = (bool*)((char*)config + e.offset);
			r.add(a);
		}else if (e.type == Kaba::TypeString){
			AutoConfigDataString *a = new AutoConfigDataString(e.name);
			a->value = (string*)((char*)config + e.offset);
			r.add(a);
		}else if (e.type->name == "SampleRef*"){
			AutoConfigDataSampleRef *a = new AutoConfigDataSampleRef(e.name, session);
			a->value = (SampleRef**)((char*)config + e.offset);
			r.add(a);
		}else if (e.type->name == "Device*"){
			AutoConfigDataDevice *a = new AutoConfigDataDevice(e.name, session);
			a->value = (Device**)((char*)config + e.offset);
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

AutoConfigPanel::AutoConfigPanel(Array<AutoConfigData*> &_aa, Module *_c) :
	ConfigPanel(_c)
{
	aa = _aa;
	add_grid("", 0, 0, "grid");
	set_target("grid");
	foreachi(AutoConfigData *a, aa, i){
		set_target("grid");
		add_label(a->label, 0, i, "");
		add_label(a->unit, 2, i, "");
		a->add_gui(this, i, [=]{ on_change(); });
	}
}

AutoConfigPanel::~AutoConfigPanel()
{
	for (auto a: aa)
		delete a;
}
void AutoConfigPanel::on_change()
{
	for (auto a: aa)
		a->get_value();
	changed();
}

void AutoConfigPanel::update()
{
	for (auto a: aa)
		a->set_value();
}

