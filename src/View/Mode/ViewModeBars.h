/*
 * ViewModeBars.h
 *
 *  Created on: 13.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEBARS_H_
#define SRC_VIEW_MODE_VIEWMODEBARS_H_

#include "ViewModeDefault.h"
#include "../../Data/Range.h"
#include "../../lib/base/base.h"

class ViewModeBars : public ViewModeDefault
{
public:
	ViewModeBars(AudioView *view);
	virtual ~ViewModeBars();

	virtual void onLeftButtonUp();
	virtual void onMouseMove();

	virtual void drawPost(HuiPainter *c);

	void startScaling(const Array<int> &sel);
	void performScale();

	bool scaling;
	bool scaling_change;
	bool modify_midi;
	Array<int> scaling_sel;
	Range scaling_range_orig;
};

#endif /* SRC_VIEW_MODE_VIEWMODEBARS_H_ */
