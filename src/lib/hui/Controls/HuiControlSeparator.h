/*
 * HuiControlSeparator.h
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#ifndef HUICONTROLSEPARATOR_H_
#define HUICONTROLSEPARATOR_H_

#include "HuiControl.h"


class HuiControlSeparator : public HuiControl
{
public:
	HuiControlSeparator(const string &text, const string &id);
	virtual ~HuiControlSeparator();
};

#endif /* HUICONTROLSEPARATOR_H_ */
