/*
 * ViewModeCurve.h
 *
 *  Created on: 14.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODECURVE_H_
#define SRC_VIEW_MODE_VIEWMODECURVE_H_

#include "ViewModeDefault.h"

class Curve;

class ViewModeCurve : public ViewModeDefault
{
public:
	ViewModeCurve(AudioView *view);
	virtual ~ViewModeCurve();

	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();
	virtual void onMouseMove();
	virtual void onKeyDown(int k);

	virtual void drawTrackData(HuiPainter *c, AudioViewTrack *t);

	virtual Selection getHover();

	Curve *curve;
	void setCurve(Curve *c);
};

#endif /* SRC_VIEW_MODE_VIEWMODECURVE_H_ */
