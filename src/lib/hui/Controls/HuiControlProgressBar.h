/*
 * HuiControlProgressBar.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLPROGRESSBAR_H_
#define HUICONTROLPROGRESSBAR_H_

#include "HuiControl.h"


class HuiControlProgressBar : public HuiControl
{
public:
	HuiControlProgressBar(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual float getFloat();
	virtual void __setFloat(float f);
};

#endif /* HUICONTROLPROGRESSBAR_H_ */
