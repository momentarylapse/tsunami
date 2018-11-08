/*
 * ControlComboBox.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLCOMBOBOX_H_
#define CONTROLCOMBOBOX_H_

#include "Control.h"

namespace hui
{


class ControlComboBox : public Control
{
public:
	ControlComboBox(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void __add_string(const string &str) override;
	void __set_int(int i) override;
	int get_int() override;
	void __reset() override;

	bool editable;
};

};

#endif /* CONTROLCOMBOBOX_H_ */
