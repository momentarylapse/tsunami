/*
 * ControlPaned.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef CONTROLPANED_H_
#define CONTROLPANED_H_

#include "Control.h"

namespace hui {

class ControlPaned : public Control {
public:
	ControlPaned(const string &text, const string &id);

	void add_child(shared<Control> child, int x, int y) override;
	void remove_child(Control *child) override;

	int get_int() override;
	void __set_int(int i) override;
};

};

#endif /* CONTROLPANED_H_ */
