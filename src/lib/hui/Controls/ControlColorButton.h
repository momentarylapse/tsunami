/*
 * ControlColorButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLCOLORBUTTON_H_
#define CONTROLCOLORBUTTON_H_

#include "Control.h"

namespace hui
{


class ControlColorButton : public Control
{
public:
	ControlColorButton(const string &text, const string &id);

	void __set_option(const string &op, const string &value) override;
	void __set_color(const color &c) override;
	color get_color() override;
};

};

#endif /* CONTROLCOLORBUTTON_H_ */
