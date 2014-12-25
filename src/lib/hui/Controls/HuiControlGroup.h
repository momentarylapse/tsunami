/*
 * HuiControlGroup.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLGROUP_H_
#define HUICONTROLGROUP_H_

#include "HuiControl.h"


class HuiControlGroup : public HuiControl
{
public:
	HuiControlGroup(const string &text, const string &id);

	virtual void add(HuiControl *child, int x, int y);
};

#endif /* HUICONTROLGROUP_H_ */
