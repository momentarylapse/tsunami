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
	string get_string() override;
	void __set_string(const string &str) override;
	void __check(bool checked) override;
	bool is_checked() override;
};

};

#endif /* CONTROLRADIOBUTTON_H_ */
