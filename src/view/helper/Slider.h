/*
 * Slider.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SLIDER_H_
#define SLIDER_H_

#include "../../lib/base/callable.h"
#include "../../lib/hui/hui.h"
#include "../../lib/pattern/Observable.h"


class Slider : public obs::Node<VirtualBase> {
public:
	using Callback = std::function<void(float)>;

	Slider(hui::Panel *_panel, const string &_id_slider, const string &_id_edit);
	~Slider();

	obs::xsource<float> out_value{this, "value"};

	void __init_ext__(hui::Panel *_panel, const string &_id_slider, const string &_id_edit, Callable<void()> *_func);
	void __delete__();

	void set(float value);
	float get();

	void set_scale(float factor);
	void set_range(float min, float max, float step);
	void set_slider_range(float min, float max);
	void enable(bool enabled);

	enum Mode {
		LINEAR,
		EXPONENTIAL,
		SQUARE
	};
	void set_mode(Mode m);

private:
	void set_slide(float f);
	void on_slide();
	void on_edit();

	string id_slider, id_edit;
	int event_handler_id[2];
	float value;
	float value_min, value_max, value_step;
	float value_min_slider, value_max_slider;
	float factor;
	Mode mode;
	hui::Panel *panel;
};

#endif /* SLIDER_H_ */
