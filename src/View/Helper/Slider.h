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
	Slider(HuiWindow *_win, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, hui_callback *_func, float _value);
	Slider(HuiWindow *_win, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, void (HuiEventHandler::*_func)(), float _value, HuiEventHandler *_handler = NULL);
	Slider(HuiWindow *_win, const string &_id_slider, const string &_id_edit, float _v_min, float _v_max, float _factor, hui_kaba_callback *_func, float _value, HuiEventHandler *_handler = NULL);
	virtual ~Slider();

	void Set(float value);
	float Get();
	void Enable(bool enabled);
	bool Match(const string &id);

	void OnSlide();
	void OnEdit();

private:
	string id_slider, id_edit;
	float value_min, value_max;
	float factor;
	HuiWindow *win;
	HuiCallback func;
};

#endif /* SLIDER_H_ */
