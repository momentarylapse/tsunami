/*
 * Slider.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SLIDER_H_
#define SLIDER_H_

#include "../../lib/hui/hui.h"

class Slider : public HuiEventHandler
{
public:
	Slider();
	Slider(HuiPanel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, const HuiCallback &func, float _value);
	Slider(HuiPanel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, hui_kaba_member_callback *_func, float _value);
	virtual ~Slider();

	void _cdecl __init_ext__(HuiPanel *_panel, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, hui_kaba_member_callback *_func, float _value);
	virtual void _cdecl __delete__();

	void _cdecl set(float value);
	float _cdecl get();
	void _cdecl enable(bool enabled);

	void onSlide();
	void onEdit();

private:
	string id_slider, id_edit;
	float value_min, value_max;
	float factor;
	HuiPanel *panel;
	HuiCallback func;
};

#endif /* SLIDER_H_ */
