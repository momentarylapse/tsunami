/*
 * HuiControlDrawingArea.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLDRAWINGAREA_H_
#define HUICONTROLDRAWINGAREA_H_

#include "HuiControl.h"


class HuiControlDrawingArea : public HuiControl
{
public:
	HuiControlDrawingArea(const string &text, const string &id);
	virtual ~HuiControlDrawingArea();

	bool grab_focus;
};

#endif /* HUICONTROLDRAWINGAREA_H_ */
