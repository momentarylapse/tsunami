/*
 * ControlMultilineEdit.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef CONTROLMULTILINEEDIT_H_
#define CONTROLMULTILINEEDIT_H_

#include "Control.h"

namespace hui
{

class ControlMultilineEdit : public Control
{
public:
	ControlMultilineEdit(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void __add_string(const string &str) override;
	void __reset() override;
	void __set_option(const string &op, const string &value) override;

	void set_tab_size(int tab_size);

	bool handle_keys;
};

};

#endif /* CONTROLMULTILINEEDIT_H_ */
