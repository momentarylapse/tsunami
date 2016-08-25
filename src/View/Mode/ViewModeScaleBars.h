/*
 * ViewModeScaleBars.h
 *
 *  Created on: 13.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODESCALEBARS_H_
#define SRC_VIEW_MODE_VIEWMODESCALEBARS_H_

#include "ViewModeDefault.h"
#include "../../Data/Range.h"
#include "../../lib/base/base.h"

class ViewModeScaleBars : public ViewModeDefault
{
public:
	ViewModeScaleBars(AudioView *view);
	virtual ~ViewModeScaleBars();

	virtual void onLeftButtonUp();
	virtual void onRightButtonDown();
	virtual void onMouseMove();
	virtual void onKeyDown(int k);

	virtual void drawPost(Painter *c);

	void startScaling(const Array<int> &sel);
	void performScale();

	bool scaling_change;
	Array<int> scaling_sel;
	Range scaling_range_orig;
};

#endif /* SRC_VIEW_MODE_VIEWMODESCALEBARS_H_ */
