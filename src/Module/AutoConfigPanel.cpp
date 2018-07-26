/*
 * AutoConfigPanel.cpp
 *
 *  Created on: 08.10.2017
 *      Author: michi
 */


#include "AutoConfigPanel.h"
#include "Module.h"
#include "../View/Helper/Slider.h"
#include "../Data/Midi/MidiData.h"
#include "../lib/kaba/kaba.h"


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
		value = nullptr;
		slider = nullptr;
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
		p->addGrid("", 1, i, "grid-" + i);
		p->setTarget("grid-" + i);
		p->addSlider("!width=150,expandx", 0, 0, "slider-" + i);
		p->addSpinButton(f2s(*value, 6), 1, 0, "spin-" + i);
		p->setOptions("spin-" + i, format("range=%f:%f:%f", min*factor, max*factor, step));
		p->addLabel(unit, 2, 0, "");
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
		value = nullptr;
		panel = nullptr;
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
		p->addSpinButton("!width=150,expandx\\" + i2s(*value), 1, i, id);
		p->setOptions(id, format("range=%d:%d", min, max));
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
		value = nullptr;
		panel = nullptr;
	}
	virtual ~AutoConfigDataPitch()
	{}
	virtual void parse(const string &s)
	{}
	virtual void add_gui(ConfigPanel *p, int i, const hui::Callback &callback)
	{
		id = "pitch-" + i;
		panel = p;
		p->addComboBox("!width=150,expandx", 1, i, id);
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
		value = nullptr;
		panel = nullptr;
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
		p->addEdit("!width=150,expandx\\" + *value, 1, i, id);
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

Array<AutoConfigData*> get_auto_conf(ModuleConfiguration *config)
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

AutoConfigPanel::AutoConfigPanel(Array<AutoConfigData*> &_aa, Module *_c) :
	ConfigPanel(_c)
{
	aa = _aa;
	addGrid("", 0, 0, "grid");
	setTarget("grid");
	foreachi(AutoConfigData *a, aa, i){
		setTarget("grid");
		addLabel(a->label, 0, i, "");
		a->add_gui(this, i, std::bind(&AutoConfigPanel::onChange, this));
	}
}

AutoConfigPanel::~AutoConfigPanel()
{
	for (auto a: aa)
		delete a;
}
void AutoConfigPanel::onChange()
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

