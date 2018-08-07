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
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void setImage(const string &str);
	virtual void __setOption(const string &op, const string &value);

	Menu *menu;
};

};

#endif /* SRC_LIB_HUI_CONTROLS_CONTROLMENUBUTTON_H_ */
