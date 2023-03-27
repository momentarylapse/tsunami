/*
 * ControlExpander.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef CONTROLEXPANDER_H_
#define CONTROLEXPANDER_H_

#include "Control.h"

namespace hui {

class ControlExpander : public Control {
public:
	ControlExpander(const string &text, const string &id);

	void expand(int row, bool expand) override;
	bool is_expanded(int row) override;

	void add_child(shared<Control> child, int x, int y) override;
	void remove_child(Control *child) override;

	void __set_option(const string& op, const string& value) override;

	GtkWidget *revealer;
	GtkWidget *expander;
};

};

#endif /* CONTROLEXPANDER_H_ */
