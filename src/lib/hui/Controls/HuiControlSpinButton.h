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
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual float getFloat();
	virtual void __setFloat(float f);
	virtual void __setOption(const string &op, const string &value);
};

#endif /* HUICONTROLSPINBUTTON_H_ */
