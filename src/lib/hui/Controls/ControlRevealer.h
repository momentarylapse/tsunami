/*
 * ControlRevealer.h
 *
 *  Created on: 17.05.2015
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CONTROLS_CONTROLREVEALER_H_
#define SRC_LIB_HUI_CONTROLS_CONTROLREVEALER_H_

#include "Control.h"

namespace hui
{

class ControlRevealer : public Control
{
public:
	ControlRevealer(const string &text, const string &id);

	virtual void add(Control *child, int x, int y);

	virtual void reveal(bool reveal);
	virtual bool isRevealed();
};

};

#endif /* SRC_LIB_HUI_CONTROLS_CONTROLREVEALER_H_ */
