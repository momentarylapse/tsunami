/*
 * Slider.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Slider.h"
#include "../../lib/kaba/kaba.h"

Slider::Slider()
{
	value_min = 0;
	value_max = 0;
	factor = 1;
	panel = nullptr;
	event_handler_id[0] = -1;
	event_handler_id[1] = -1;
}


Slider::Slider(hui::Panel *_panel, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, const hui::Callback &_func, float _value)
{
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = _func;

	event_handler_id[0] = panel->event(id_slider, [=]{ on_slide(); });
	event_handler_id[1] = panel->event(id_edit, [=]{ on_edit(); });

	set(_value);
}



Slider::Slider(hui::Panel *_panel, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, Kaba::Function *_func, float _value)
{
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	auto *ff = (hui::kaba_member_callback*)_func->address;
	func = [=]{ ff(panel); };

	event_handler_id[0] = panel->event(id_slider, [=]{ on_slide(); });
	event_handler_id[1] = panel->event(id_edit, [=]{ on_edit(); });

	set(_value);
}


Slider::~Slider()
{
	if (panel){
		panel->remove_event_handler(event_handler_id[0]);
		panel->remove_event_handler(event_handler_id[1]);
	}
}

void Slider::__init_ext__(hui::Panel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, Kaba::Function *_func, float _value)
{
	new(this) Slider(_panel, _id_slider, _id_edit, _v_min, _v_max, _factor, _func, _value);
}

void Slider::__delete__()
{
	this->Slider::~Slider();
}

void Slider::set(float value)
{
	panel->set_float(id_slider, (value - value_min) / (value_max - value_min));
	panel->set_float(id_edit, value * factor);
}



float Slider::get()
{
	return panel->get_float(id_edit) / factor;
}


void Slider::enable(bool enabled)
{
	panel->enable(id_slider, enabled);
	panel->enable(id_edit, enabled);
}


void Slider::on_slide()
{
	float value = value_min + panel->get_float(id_slider) * (value_max - value_min);
	panel->set_float(id_edit, value * factor);
	func();
}

void Slider::on_edit()
{
	float value = panel->get_float(id_edit) / factor;
	panel->set_float(id_slider, (value - value_min) / (value_max - value_min));
	func();
}

