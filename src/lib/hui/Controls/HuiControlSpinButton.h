/*
 * HuiControlSpinButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLSPINBUTTON_H_
#define HUICONTROLSPINBUTTON_H_

#include "HuiControl.h"


class HuiControlSpinButton : public HuiControl
{
public:
	HuiControlSpinButton(const string &text, const string &id);
	virtual ~HuiControlSpinButton();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void __SetInt(int i);
	virtual int GetInt();
	virtual float GetFloat();
	virtual void __SetFloat(float f);
	virtual void __SetOption(const string &op, const string &value);
};

#endif /* HUICONTROLSPINBUTTON_H_ */
