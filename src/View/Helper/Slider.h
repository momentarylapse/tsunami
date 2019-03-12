/*
 * Slider.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SLIDER_H_
#define SLIDER_H_

#include "../../lib/hui/hui.h"

class Slider : public hui::EventHandler
{
public:
	Slider();
	Slider(hui::Panel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, const hui::Callback &func, float _value);
	Slider(hui::Panel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, hui::kaba_member_callback *_func, float _value);
	virtual ~Slider();

	void _cdecl __init_ext__(hui::Panel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, hui::kaba_member_callback *_func, float _value);
	virtual void _cdecl __delete__();

	void _cdecl set(float value);
	float _cdecl get();
	void _cdecl enable(bool enabled);

	void on_slide();
	void on_edit();

private:
	string id_slider, id_edit;
	int event_handler_id[2];
	float value_min, value_max;
	float factor;
	hui::Panel *panel;
	hui::Callback func;
};

#endif /* SLIDER_H_ */
