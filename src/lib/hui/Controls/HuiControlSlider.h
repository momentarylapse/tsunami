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
	virtual float getFloat();
	virtual void __setFloat(float f);
	virtual void __addString(const string &s);
	virtual void __setOption(const string &op, const string &value);
	bool vertical;
};

#endif /* HUICONTROLSLIDER_H_ */
