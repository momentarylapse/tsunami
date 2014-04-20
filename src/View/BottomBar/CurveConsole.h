/*
 * CurveConsole.h
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#ifndef CURVECONSOLE_H_
#define CURVECONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"
#include "../../lib/math/math.h"

class AudioFile;
class AudioView;
class Curve;

class CurveConsole : public BottomBarConsole, public Observer
{
public:
	CurveConsole(AudioView *view, AudioFile *audio);
	virtual ~CurveConsole();

	virtual void OnUpdate(Observable *o, const string &message);

	void updateList();
	void onAdd();
	void onTarget();
	void onListSelect();
	void onDraw();
	void onLeftButtonDown();
	void onLeftButtonUp();
	void onMouseMove();

	AudioFile *audio;
	AudioView *view;
	Curve *curve;

	rect curve_rect;

	int hover, selected;

	float sample2screen(float pos);
	float screen2sample(float x);
	float value2screen(float value);
	float screen2value(float y);
};

#endif /* CURVECONSOLE_H_ */
