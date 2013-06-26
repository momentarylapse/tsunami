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
	virtual ~HuiControlButton();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void SetImage(const string &str);
};

#endif /* HUICONTROLBUTTON_H_ */
