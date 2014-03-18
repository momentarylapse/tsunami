/*
 * HuiControlRadioButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLRADIOBUTTON_H_
#define HUICONTROLRADIOBUTTON_H_

#include "HuiControl.h"


class HuiControlRadioButton : public HuiControl
{
public:
	HuiControlRadioButton(const string &text, const string &id, HuiPanel *panel);
	virtual ~HuiControlRadioButton();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void __Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUICONTROLRADIOBUTTON_H_ */
