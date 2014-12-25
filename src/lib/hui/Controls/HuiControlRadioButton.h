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
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __check(bool checked);
	virtual bool isChecked();
};

#endif /* HUICONTROLRADIOBUTTON_H_ */
