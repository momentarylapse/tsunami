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
	virtual ~HuiControlProgressBar();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual float GetFloat();
	virtual void __SetFloat(float f);
};

#endif /* HUICONTROLPROGRESSBAR_H_ */
