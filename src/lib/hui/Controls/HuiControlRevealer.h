/*
 * HuiControlRevealer.h
 *
 *  Created on: 17.05.2015
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CONTROLS_HUICONTROLREVEALER_H_
#define SRC_LIB_HUI_CONTROLS_HUICONTROLREVEALER_H_

#include "HuiControl.h"


class HuiControlRevealer : public HuiControl
{
public:
	HuiControlRevealer(const string &text, const string &id);

	virtual void add(HuiControl *child, int x, int y);

	virtual void reveal(bool reveal);
	virtual bool isRevealed();
};

#endif /* SRC_LIB_HUI_CONTROLS_HUICONTROLREVEALER_H_ */
