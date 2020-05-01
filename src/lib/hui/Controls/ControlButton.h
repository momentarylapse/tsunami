/*
 * ControlButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLBUTTON_H_
#define CONTROLBUTTON_H_

#include "Control.h"

namespace hui
{

class Panel;


class ControlButton : public Control {
public:
	ControlButton(const string &text, const string &id, Panel *panel);
	string get_string() override;
	void __set_string(const string &str) override;
	void set_image(const string &str) override;
	void __set_option(const string &op, const string &value) override;
};

};

#endif /* CONTROLBUTTON_H_ */
