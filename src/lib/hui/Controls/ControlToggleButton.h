/*
 * ControlToggleButton.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef CONTROLTOGGLEBUTTON_H_
#define CONTROLTOGGLEBUTTON_H_

#include "Control.h"

namespace hui
{

class ControlToggleButton : public Control
{
public:
	ControlToggleButton(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void set_image(const string &str) override;
	void __check(bool checked) override;
	bool is_checked() override;
	void __set_option(const string &op, const string &value) override;
};

};

#endif /* CONTROLTOGGLEBUTTON_H_ */
