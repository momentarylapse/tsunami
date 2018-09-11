/*
 * ViewPort.h
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_VIEWPORT_H_
#define SRC_VIEW_VIEWPORT_H_

class AudioView;
class Range;
class rect;

class ViewPort
{
public:
	ViewPort(AudioView *v);

	static const float BORDER_FACTOR;
	static const float BORDER_FACTOR_RIGHT;

	AudioView *view;

	double pos;
	double pos_pre_animation;
	double pos_target;
	double animation_time;
	double animation_non_linearity;
	double scale;
	Range range();

	void update(float dt);
	bool needs_update();

	double screen2sample(double x);
	double sample2screen(double s);
	double dsample2screen(double ds);
	double dscreen2sample(double dx);

	void zoom(float f);
	void move(float dpos);
	void set_target(float pos, float nonlin);

	void make_sample_visible(int sample, int samples_ahead);
	rect nice_mapping_area();
	void show(Range &r);
};

#endif /* SRC_VIEW_VIEWPORT_H_ */
