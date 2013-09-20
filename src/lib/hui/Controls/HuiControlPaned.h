/*
 * HuiControlPaned.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef HUICONTROLPANED_H_
#define HUICONTROLPANED_H_

#include "HuiControl.h"

class HuiControlPaned : public HuiControl
{
public:
	HuiControlPaned(const string &text, const string &id);
	virtual ~HuiControlPaned();

	virtual void add(HuiControl *child, int x, int y);
};

#endif /* HUICONTROLPANED_H_ */
