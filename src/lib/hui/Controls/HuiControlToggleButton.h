/*
 * HuiControlToggleButton.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLTOGGLEBUTTON_H_
#define HUICONTROLTOGGLEBUTTON_H_

#include "HuiControl.h"


class HuiControlToggleButton : public HuiControl
{
public:
	HuiControlToggleButton(const string &text, const string &id);
	virtual ~HuiControlToggleButton();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void SetImage(const string &str);
	virtual void __Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUICONTROLTOGGLEBUTTON_H_ */
