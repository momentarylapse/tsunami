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
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void setImage(const string &str);
	virtual void __check(bool checked);
	virtual bool isChecked();
};

#endif /* HUICONTROLTOGGLEBUTTON_H_ */
