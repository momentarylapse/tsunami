/*
 * Slider.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Slider.h"



Slider::Slider(HuiPanel *_panel, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, hui_callback *_func, float _value)
{
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = HuiCallback(_func);

	panel->EventM(id_slider, this, &Slider::OnSlide);
	panel->EventM(id_edit, this, &Slider::OnEdit);

	Set(_value);
}



Slider::Slider(HuiPanel *_panel, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, void(HuiEventHandler::*_func)(), float _value, HuiEventHandler *_handler)
{
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = HuiCallback(_handler ? _handler : panel, _func);

	panel->EventM(id_slider, this, &Slider::OnSlide);
	panel->EventM(id_edit, this, &Slider::OnEdit);

	Set(_value);
}

Slider::Slider(HuiPanel *_panel, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, hui_kaba_callback *_func, float _value, HuiEventHandler *_handler)
{
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = HuiCallback(_handler ? _handler : panel, _func);

	panel->EventM(id_slider, this, &Slider::OnSlide);
	panel->EventM(id_edit, this, &Slider::OnEdit);

	Set(_value);
}


Slider::~Slider()
{
}

void Slider::Set(float value)
{
	panel->SetFloat(id_slider, (value - value_min) / (value_max - value_min));
	panel->SetFloat(id_edit, value * factor);
}



float Slider::Get()
{
	return panel->GetFloat(id_edit) / factor;
}


void Slider::Enable(bool enabled)
{
	panel->Enable(id_slider, enabled);
	panel->Enable(id_edit, enabled);
}


bool Slider::Match(const string &id)
{
	return ((id == id_slider) or (id == id_edit));
}


void Slider::OnSlide()
{
	float value = value_min + panel->GetFloat("") * (value_max - value_min);
	panel->SetFloat(id_edit, value * factor);
	func.call();
}

void Slider::OnEdit()
{
	float value = panel->GetFloat("") / factor;
	panel->SetFloat(id_slider, (value - value_min) / (value_max - value_min));
	func.call();
}

