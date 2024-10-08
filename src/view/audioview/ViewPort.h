/*
 * ViewPort.h
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_VIEWPORT_H_
#define SRC_VIEW_VIEWPORT_H_

#include "../../lib/math/rect.h"
#include "../../lib/pattern/Observable.h"

namespace tsunami {

class Range;
class Song;

class ViewPort : public obs::Node<VirtualBase> {
public:
	ViewPort();
	void __init__();

	static const float BorderFactor;
	static const float BorderFactorRight;

	double pos;
	double pos_pre_animation;
	double pos_target;
	double animation_time;
	double animation_non_linearity;
	double pixels_per_sample;
	Range range() const;
	void set_range(const Range &r);

	float scale_y;

	void update(float dt);
	bool needs_update();

	double screen2sample(double x);
	double sample2screen(double s);
	double dsample2screen(double ds);
	double dscreen2sample(double dx);
	float screen2sample_f(float x);
	float sample2screen_f(float s);
	void range2screen(const Range &r, float &x1, float &x2);
	void range2screen_clip(const Range &r, const rect &area, float &x1, float &x2);

	void zoom(float f, float mx);
	void move(float dpos);
	void set_target(float pos, float nonlin);
	void dirty_jump(float pos);

	rect area;

	void make_sample_visible(int sample, int samples_ahead);
	rect nice_mapping_area();
	void show(const Range &r);
};

}

#endif /* SRC_VIEW_VIEWPORT_H_ */
