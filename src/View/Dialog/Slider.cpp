/*
 * Slider.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Slider.h"



Slider::Slider(CHuiWindow *_win, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, hui_callback *_func, float _value)
{
	win = _win;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = _func;
	member_func = NULL;

	win->EventM(id_slider, this, (void(HuiEventHandler::*)())&Slider::OnSlide);
	win->EventM(id_edit, this, (void(HuiEventHandler::*)())&Slider::OnEdit);

	Set(_value);
}



Slider::Slider(CHuiWindow *_win, const string & _id_slider, const string & _id_edit, float _v_min, float _v_max, float _factor, void(HuiEventHandler::*_func)(), float _value)
{
	win = _win;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = _v_min;
	value_max = _v_max;
	factor = _factor;
	func = NULL;
	member_func = _func;

	win->EventM(id_slider, this, (void(HuiEventHandler::*)())&Slider::OnSlide);
	win->EventM(id_edit, this, (void(HuiEventHandler::*)())&Slider::OnEdit);

	Set(_value);
}


Slider::~Slider()
{
	// TODO Auto-generated destructor stub
}

void Slider::Set(float value)
{
	win->SetFloat(id_slider, (value - value_min) / (value_max - value_min));
	win->SetFloat(id_edit, value * factor);
}



float Slider::Get()
{
	return win->GetFloat(id_edit) / factor;
}


void Slider::Enabled(bool enabled)
{
	win->Enable(id_slider, enabled);
	win->Enable(id_edit, enabled);
}


void Slider::OnSlide()
{
	float value = value_min + win->GetFloat("") * (value_max - value_min);
	win->SetFloat(id_edit, value * factor);
	if (func)
		func();
	if (member_func)
		(win->*member_func)();
}

void Slider::OnEdit()
{
	float value = win->GetFloat("") / factor;
	win->SetFloat(id_slider, (value - value_min) / (value_max - value_min));
	if (func)
		func();
	if (member_func)
		(win->*member_func)();
}

