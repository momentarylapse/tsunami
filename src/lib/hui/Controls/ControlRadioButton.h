/*
 * ControlRadioButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLRADIOBUTTON_H_
#define CONTROLRADIOBUTTON_H_

#include "Control.h"

namespace hui
{

class ControlRadioButton : public Control
{
public:
	ControlRadioButton(const string &text, const string &id, Panel *panel);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __check(bool checked);
	virtual bool isChecked();
};

};

#endif /* CONTROLRADIOBUTTON_H_ */
