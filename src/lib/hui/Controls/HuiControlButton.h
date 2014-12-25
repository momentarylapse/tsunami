/*
 * HuiControlButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLBUTTON_H_
#define HUICONTROLBUTTON_H_

#include "HuiControl.h"


class HuiControlButton : public HuiControl
{
public:
	HuiControlButton(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void setImage(const string &str);
	virtual void __setOption(const string &op, const string &value);
};

#endif /* HUICONTROLBUTTON_H_ */
