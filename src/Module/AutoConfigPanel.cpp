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


string to_camel_case(const string &s) {
	string r;
	bool next_upper = true;
	for (int i=0; i<s.num; i++) {
		if (s[i] == '_') {
			next_upper = true;
			continue;
		} else {
			if (next_upper)
				r += s.sub(i, i+1).upper();
			else
				r += s.sub(i, i+1);
			next_upper = false;
		}
	}
	return r;
}

string shorten(const string &s, int max_length);

class AutoConfigData : public VirtualBase {
public:
	string name, label, unit;
	AutoConfigData(const string &_name) {
		name = _name;
		label = to_camel_case(_name);
	}
	virtual ~AutoConfigData() {}

	bool name_match(const string &const_name) {
		return (("AUTO_CONFIG_" + name).upper().replace("_", "") == const_name.upper().replace("_", ""));
	}
	virtual void parse(const string &s) = 0; // before a gui exists!
	virtual void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) = 0;
	virtual void value_from_gui() = 0;
	virtual void value_to_gui() = 0;
};

class AutoConfigDataFloat : public AutoConfigData {
public:
	float min, max, step, factor;
	float *value;
	owned<Slider> slider;
	AutoConfigDataFloat(const string &_name) : AutoConfigData(_name) {
		min = -1000000;
		max = 1000000;
		step = 0.001f;
		factor = 1;
		value = nullptr;
	}
	void parse(const string &s) override {
		auto p = s.explode(":");
		if (p.num == 5) {
			min = p[0]._float();
			max = p[1]._float();
			step = p[2]._float();
			factor = p[3]._float();
			unit = p[4];
		} else {
			msg_write("required format: min:max:step:factor:unit");
		}

	}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override {
		p->add_grid("", 1, i, "grid-" + i2s(i));
		p->set_target("grid-" + i2s(i));
		p->add_slider("!expandx", 0, 0, "slider-" + i2s(i));
		p->add_spin_button("!width=50\\" + f2s(*value, 6), 1, 0, "spin-" + i2s(i));
		p->set_options("spin-" + i2s(i), format("range=%f:%f:%f", min*factor, max*factor, step));
		//p->addLabel(unit, 2, 0, "");
		slider = new Slider(p, "slider-" + i2s(i), "spin-" + i2s(i), min, max, factor, callback, *value);
	}
	void value_from_gui() override {
		*value = slider->get();
	}
	void value_to_gui() override {
		slider->set(*value);
	}
};

class AutoConfigDataBool : public AutoConfigData {
public:
	bool *value;
	ConfigPanel *panel;
	string id;
	AutoConfigDataBool(const string &_name) : AutoConfigData(_name) {
		value = nullptr;
		panel = nullptr;
	}
	~AutoConfigDataBool() override {};
	void parse(const string &s) override {};
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override {
		id = "check-" + i2s(i);
		panel = p;
		p->add_check_box("!width=150,expandx", 1, i, id);
		p->event(id, callback);
	}
	void value_from_gui() override {
		*value = panel->is_checked(id);
	}
	void value_to_gui() override {
		panel->check(id, *value);
	}
};

class AutoConfigDataInt : public AutoConfigData {
public:
	int min, max;
	int *value;
	ConfigPanel *panel;
	string id;
	AutoConfigDataInt(const string &_name) : AutoConfigData(_name) {
		min = 0;
		max = 1000000;
		value = nullptr;
		panel = nullptr;
	}
	void parse(const string &s) override {
		auto p = s.explode(":");
		if (p.num == 2) {
			min = p[0]._int();
			max = p[1]._int();
		} else {
			msg_write("required format: min:max");
		}
	}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override {
		id = "spin-" + i2s(i);
		panel = p;
		p->add_spin_button("!width=150,expandx\\" + i2s(*value), 1, i, id);
		p->set_options(id, format("range=%d:%d", min, max));
		p->event(id, callback);
	}
	void value_from_gui() override {
		*value = panel->get_int(id);
	}
	void value_to_gui() override {
		panel->set_int(id, *value);
	}
};

class AutoConfigDataPitch : public AutoConfigData {
public:
	float *value;
	string id;
	string id_freq;
	string id_mode;
	string id_hz;
	ConfigPanel *panel;
	hui::Callback callback;
	AutoConfigDataPitch(const string &_name) : AutoConfigData(_name) {
		value = nullptr;
		panel = nullptr;
		callback = nullptr;
	}
	void parse(const string &s) override {}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &cb) override {
		callback = cb;
		id = "pitch-" + i2s(i);
		id_freq = "freq-" + i2s(i);
		id_hz = "hz-" + i2s(i);
		id_mode = "mode-" + i2s(i);
		string id_grid = "grid-" + i2s(i);
		panel = p;
		p->add_grid("", 1, i, id_grid);
		p->add_toggle_button("f", 2, i, id_mode);
		p->set_tooltip(id_mode, _("mode: frequency/pitch"));
		p->set_target(id_grid);
		p->add_combo_box("!width=150,expandx", 0, 0, id);
		for (int j=0; j<MAX_PITCH; j++)
			p->add_string(id, pitch_name(j));
		p->add_spin_button("!range=0:10000:0.1,expandx", 1, 0, id_freq);
		p->add_label("Hz", 2, 0, id_hz);
		on_mode();
		p->set_int(id, (int)*value);
		p->set_float(id_freq, pitch_to_freq(*value));
		p->event(id, [=]{ on_pitch(); });
		p->event(id_freq, [=]{ on_freq(); });
		p->event(id_mode, [=]{ on_mode(); });
	}
	void value_from_gui() override {
		bool m = panel->is_checked(id_mode);
		if (m)
			*value = freq_to_pitch(panel->get_float(id_freq));
		else
			*value = (float)panel->get_int(id);
	}
	void value_to_gui() override {
		panel->set_int(id, (int)*value);
		panel->set_float(id_freq, pitch_to_freq(*value));
	}
	void on_pitch() {
		float p = panel->get_float(id);
		panel->set_float(id_freq, pitch_to_freq(p));
		callback();
	}
	void on_freq() {
		float f = panel->get_float(id_freq);
		panel->set_int(id, (int)freq_to_pitch(f));
		callback();
	}
	void on_mode() {
		bool m = panel->is_checked(id_mode);
		panel->hide_control(id, m);
		panel->hide_control(id_freq, !m);
		panel->hide_control(id_hz, !m);
	}
};

class AutoConfigDataString : public AutoConfigData {
public:
	string *value;
	string id;
	ConfigPanel *panel;
	Array<string> choices;
	AutoConfigDataString(const string &_name) : AutoConfigData(_name) {
		value = nullptr;
		panel = nullptr;
	}
	void parse(const string &s) override {
		if (s.find("|", 0) >= 0)
			choices = s.explode("|");
	}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override {
		panel = p;
		id = "string-" + i2s(i);
		if (choices.num > 0) {
			p->add_combo_box("!expandx", 1, i, id);
			for (string &c: choices)
				p->add_string(id, c);
		} else {
			p->add_edit("!width=150,expandx\\" + *value, 1, i, id);
		}
		p->event(id, callback);
	}
	void value_from_gui() override {
		if (choices.num > 0) {
			int n = panel->get_int(id);
			if (n >= 0)
				*value = choices[n];
		} else {
			*value = panel->get_string(id);
		}
	}
	void value_to_gui() override {
		if (choices.num > 0) {
			foreachi (string &c, choices, i)
				if (c == *value)
					panel->set_int(id, i);
		} else {
			panel->set_string(id, *value);
		}
	}
};

class AutoConfigDataSampleRef : public AutoConfigData {
public:
	shared<SampleRef> *value;
	string id;
	ConfigPanel *panel;
	Session *session;
	hui::Callback callback;
	AutoConfigDataSampleRef(const string &_name, Session *_session) : AutoConfigData(_name) {
		value = nullptr;
		panel = nullptr;
		session = _session;
	}
	void parse(const string &s) override {}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &_callback) override {
		id = "sample-" + i2s(i);
		panel = p;
		p->add_button("!expandx", 1, i, id);
		value_to_gui();
		p->event(id, [=]{ on_button(); });
		callback = _callback;
	}
	void on_button() {
		Sample *old = nullptr;
		if (*value)
			old = (*value)->origin.get();
		SampleManagerConsole::select(session, panel, old, [this, old] (Sample *s) {
			if (s != old) {
				*value = nullptr;
				if (s)
					*value = s->create_ref();
				value_to_gui();
				callback();
			}
		});
	}
	void value_from_gui() override {
	}
	void value_to_gui() override {
		if (*value)
			panel->set_string(id, (*value)->origin->name);
		else
			panel->set_string(id, _(" - none -"));
	}
};

class AutoConfigDataDevice: public AutoConfigData {
public:
	Device **value;
	string id;
	ConfigPanel *panel;
	DeviceType type;
	Session *session;
	Array<Device*> list;
	AutoConfigDataDevice(const string &_name, Session *_session) : AutoConfigData(_name) {
		session = _session;
		value = nullptr;
		panel = nullptr;
		type = DeviceType::AUDIO_OUTPUT;
	}
	void parse(const string &s) override {
		type = DeviceType::AUDIO_OUTPUT;
		if (s == "audio:input")
			type = DeviceType::AUDIO_INPUT;
		if (s == "midi:input")
			type = DeviceType::MIDI_INPUT;
	}
	void update_list() {
		list = session->device_manager->good_device_list(type);
		if (panel) {
			panel->reset(id);
			for (auto *d: list)
				panel->add_string(id, shorten(d->get_name(), 42));
			panel->set_int(id, list.find(*value));
		}
	}
	void add_gui(ConfigPanel *p, int i, const hui::Callback &callback) override {
		id = "device-" + i2s(i);
		panel = p;
		p->add_combo_box("!expandx", 1, i, id);
		p->event(id, callback);
		update_list();
	}
	void value_from_gui() override {
		*value = session->device_manager->choose_device(type);
		int n = panel->get_int(id);
		if (n >= 0)
			*value = list[n];
	}
	void value_to_gui() override {
		panel->set_int(id, list.find(*value));
	}
};

Array<AutoConfigData*> get_auto_conf(ModuleConfiguration *config, Session *session) {
	auto *ps = config->kaba_class->owner;
	Array<AutoConfigData*> r;
	for (auto &e: config->kaba_class->elements) {
		if (e.type == kaba::TypeFloat32) {
			if (e.name == "pitch") {
				auto *a = new AutoConfigDataPitch(e.name);
				a->value = (float*)((char*)config + e.offset);
				r.add(a);
			} else {
				auto *a = new AutoConfigDataFloat(e.name);
				a->value = (float*)((char*)config + e.offset);
				r.add(a);
			}
		} else if (e.type == kaba::TypeInt) {
			auto *a = new AutoConfigDataInt(e.name);
			a->value = (int*)((char*)config + e.offset);
			r.add(a);
		} else if (e.type == kaba::TypeBool) {
			auto *a = new AutoConfigDataBool(e.name);
			a->value = (bool*)((char*)config + e.offset);
			r.add(a);
		} else if (e.type == kaba::TypeString) {
			auto *a = new AutoConfigDataString(e.name);
			a->value = (string*)((char*)config + e.offset);
			r.add(a);
		} else if (e.type->name == "shared SampleRef") {
			auto *a = new AutoConfigDataSampleRef(e.name, session);
			a->value = (shared<SampleRef>*)((char*)config + e.offset);
			r.add(a);
		} else if (e.type->name == "Device*") {
			auto *a = new AutoConfigDataDevice(e.name, session);
			a->value = (Device**)((char*)config + e.offset);
			r.add(a);
		}
	}

	if (ps) {
		for (auto a: r)
			a->parse(config->auto_conf(a->name));
	}

	return r;
}

AutoConfigPanel::AutoConfigPanel(Array<AutoConfigData*> &_aa, Module *_c) :
	ConfigPanel(_c) {
	aa = _aa;
	add_grid("", 0, 0, "grid");
	set_target("grid");
	foreachi(auto *a, aa, i) {
		set_target("grid");
		add_label("!right,disabled\\" + a->label, 0, i, format("label-%d", i));
		add_label(a->unit, 2, i, "");
		a->add_gui(this, i, [a,this]{
			a->value_from_gui();
			changed();
		});
	}
}

AutoConfigPanel::~AutoConfigPanel() {
	for (auto a: aa)
		delete a;
}

void AutoConfigPanel::update() {
	for (auto a: aa)
		a->value_to_gui();
}

