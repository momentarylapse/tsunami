/*
 * ViewModeCapture.h
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODECAPTURE_H_
#define SRC_VIEW_MODE_VIEWMODECAPTURE_H_

#include "ViewModeDefault.h"
#include "../../Data/Range.h"
#include "../../lib/base/base.h"

class ViewModeCapture : public ViewModeDefault
{
public:
	ViewModeCapture(AudioView *view);
	virtual ~ViewModeCapture();

	virtual Selection getHover();
	virtual void onLeftButtonDown(){}
	virtual void onLeftDoubleClick(){}
};

#endif /* SRC_VIEW_MODE_VIEWMODECAPTURE_H_ */
