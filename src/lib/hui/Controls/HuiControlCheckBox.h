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
	virtual ~HuiControlCheckBox();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void __Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUICONTROLCHECKBOX_H_ */
