/*
 * ControlSeparator.h
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#ifndef CONTROLSEPARATOR_H_
#define CONTROLSEPARATOR_H_

#include "Control.h"

namespace hui
{

class ControlSeparator : public Control
{
public:
	ControlSeparator(const string &text, const string &id);
};

};

#endif /* CONTROLSEPARATOR_H_ */
