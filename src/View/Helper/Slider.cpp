/*
 * Slider.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Slider.h"

Slider::Slider()
{
}


Slider::Slider(HuiPanel *_panel, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, const HuiCallback &_func, float _value)
{
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = _func;

	panel->event(id_slider, std::bind(&Slider::onSlide, this));
	panel->event(id_edit, std::bind(&Slider::onEdit, this));

	set(_value);
}



Slider::Slider(HuiPanel *_panel, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, hui_kaba_member_callback *_func, float _value)
{
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = std::bind(_func, panel);

	panel->event(id_slider, std::bind(&Slider::onSlide, this));
	panel->event(id_edit, std::bind(&Slider::onEdit, this));

	set(_value);
}


Slider::~Slider()
{
}

void Slider::__init_ext__(HuiPanel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, hui_kaba_member_callback *_func, float _value)
{
	new(this) Slider(_panel, _id_slider, _id_edit, _v_min, _v_max, _factor, _func, _value);
}

void Slider::__delete__()
{
	this->Slider::~Slider();
}

void Slider::set(float value)
{
	panel->setFloat(id_slider, (value - value_min) / (value_max - value_min));
	panel->setFloat(id_edit, value * factor);
}



float Slider::get()
{
	return panel->getFloat(id_edit) / factor;
}


void Slider::enable(bool enabled)
{
	panel->enable(id_slider, enabled);
	panel->enable(id_edit, enabled);
}


void Slider::onSlide()
{
	float value = value_min + panel->getFloat("") * (value_max - value_min);
	panel->setFloat(id_edit, value * factor);
	func();
}

void Slider::onEdit()
{
	float value = panel->getFloat("") / factor;
	panel->setFloat(id_slider, (value - value_min) / (value_max - value_min));
	func();
}

