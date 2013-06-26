/*
 * HuiToolItemToggleButton.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUITOOLITEMTOGGLEBUTTON_H_
#define HUITOOLITEMTOGGLEBUTTON_H_

#include "HuiControl.h"

class HuiToolItemToggleButton : public HuiControl
{
public:
	HuiToolItemToggleButton(const string &title, const string &image, const string &id);
	virtual ~HuiToolItemToggleButton();

	virtual void __Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUITOOLITEMTOGGLEBUTTON_H_ */
