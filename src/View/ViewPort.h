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

class ViewPort
{
public:
	ViewPort(AudioView *v);

	static const float BORDER_FACTOR;

	AudioView *view;

	double pos;
	double scale;
	Range range();

	double screen2sample(double x);
	double sample2screen(double s);
	double dsample2screen(double ds);

	void zoom(float f);
	void move(float dpos);

	void makeSampleVisible(int sample);
	void show(Range &r);
};

#endif /* SRC_VIEW_VIEWPORT_H_ */
