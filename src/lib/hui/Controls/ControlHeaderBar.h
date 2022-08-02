/*
 * ControlHeaderBar.h
 *
 *  Created on: 11 Feb 2022
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CONTROLS_CONTROLHEADERBAR_H_
#define SRC_LIB_HUI_CONTROLS_CONTROLHEADERBAR_H_

#include "Control.h"

namespace hui {

class ControlHeaderBar : public Control {
public:
	ControlHeaderBar(const string &text, const string &id, Panel *panel);
	void add_child(shared<Control> child, int x, int y) override;
	void remove_child(Control *child) override;

	void __set_option(const string &op, const string &value) override;
};

}

#endif /* SRC_LIB_HUI_CONTROLS_CONTROLHEADERBAR_H_ */
