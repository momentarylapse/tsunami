/*
 * HuiMenuItemToggle.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUIMENUITEMTOGGLE_H_
#define HUIMENUITEMTOGGLE_H_

#include "HuiControl.h"

class HuiMenuItemToggle : public HuiControl
{
public:
	HuiMenuItemToggle(const string &title, const string &id);
	virtual ~HuiMenuItemToggle();

	virtual void __Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUIMENUITEMTOGGLE_H_ */
