/*
 * HuiControlSlider.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLSLIDER_H_
#define HUICONTROLSLIDER_H_

#include "HuiControl.h"


class HuiControlSlider : public HuiControl
{
public:
	HuiControlSlider(const string &text, const string &id, bool vertical);
	virtual ~HuiControlSlider();
	virtual float GetFloat();
	virtual void __SetFloat(float f);
	virtual void __AddString(const string &s);
	bool vertical;
};

#endif /* HUICONTROLSLIDER_H_ */
