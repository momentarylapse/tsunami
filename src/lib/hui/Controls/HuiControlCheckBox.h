/*
 * HuiControlCheckBox.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLCHECKBOX_H_
#define HUICONTROLCHECKBOX_H_

#include "HuiControl.h"


class HuiControlCheckBox : public HuiControl
{
public:
	HuiControlCheckBox(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __check(bool checked);
	virtual bool isChecked();
};

#endif /* HUICONTROLCHECKBOX_H_ */
