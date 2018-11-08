/*
 * ControlMenuButton.h
 *
 *  Created on: 07.08.2018
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CONTROLS_CONTROLMENUBUTTON_H_
#define SRC_LIB_HUI_CONTROLS_CONTROLMENUBUTTON_H_


#include "Control.h"

namespace hui
{

class Menu;


class ControlMenuButton : public Control
{
public:
	ControlMenuButton(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void set_image(const string &str) override;
	void __set_option(const string &op, const string &value) override;

	Menu *menu;
};

};

#endif /* SRC_LIB_HUI_CONTROLS_CONTROLMENUBUTTON_H_ */
