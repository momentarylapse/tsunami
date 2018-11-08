/*
 * ControlRevealer.h
 *
 *  Created on: 17.05.2015
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CONTROLS_CONTROLREVEALER_H_
#define SRC_LIB_HUI_CONTROLS_CONTROLREVEALER_H_

#include "Control.h"

namespace hui
{

class ControlRevealer : public Control
{
public:
	ControlRevealer(const string &text, const string &id);

	void add(Control *child, int x, int y) override;

	void reveal(bool reveal) override;
	bool is_revealed() override;

	void __set_option(const string &op, const string &value) override;
};

};

#endif /* SRC_LIB_HUI_CONTROLS_CONTROLREVEALER_H_ */
