/*
 * HuiControlColorButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLCOLORBUTTON_H_
#define HUICONTROLCOLORBUTTON_H_

#include "HuiControl.h"


class HuiControlColorButton : public HuiControl
{
public:
	HuiControlColorButton(const string &text, const string &id);
	virtual ~HuiControlColorButton();

	virtual void __SetColor(const color &c);
	virtual color GetColor();
};

#endif /* HUICONTROLCOLORBUTTON_H_ */
