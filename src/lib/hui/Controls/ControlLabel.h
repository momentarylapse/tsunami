/*
 * ControlLabel.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROL_LABEL_H_
#define CONTROL_LABEL_H_

#include "Control.h"

namespace hui
{


class ControlLabel : public Control
{
public:
	ControlLabel(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void __set_option(const string &op, const string &value) override;

	string text;
	bool flag_bold, flag_italic, flag_big, flag_small, flag_underline, flag_strikeout;
};

};


#endif /* CONTROL_LABEL_H_ */
