/*
 * ControlCheckBox.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLCHECKBOX_H_
#define CONTROLCHECKBOX_H_

#include "Control.h"

namespace hui
{


class ControlCheckBox : public Control
{
public:
	ControlCheckBox(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void __check(bool checked) override;
	bool is_checked() override;
};

};

#endif /* CONTROLCHECKBOX_H_ */
