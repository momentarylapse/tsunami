/*
 * ControlSpinButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLSPINBUTTON_H_
#define CONTROLSPINBUTTON_H_

#include "Control.h"

namespace hui
{

class ControlSpinButton : public Control
{
public:
	ControlSpinButton(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void __set_int(int i) override;
	int get_int() override;
	float get_float() override;
	void __set_float(float f) override;
	void __set_option(const string &op, const string &value) override;
};

};

#endif /* CONTROLSPINBUTTON_H_ */
