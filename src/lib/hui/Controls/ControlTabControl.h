/*
 * ControlTabControl.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLTABCONTROL_H_
#define CONTROLTABCONTROL_H_

#include "Control.h"

namespace hui
{

class ControlTabControl : public Control {
public:
	ControlTabControl(const string &text, const string &id, Panel *panel);
	~ControlTabControl() override;
	string get_string() override;
	void __set_string(const string &str) override;
	void __set_int(int i) override;
	int get_int() override;
	void __add_string(const string &str) override;
	void __remove_string(int row) override;
	void __change_string(int row, const string &str) override;
	void __set_option(const string &op, const string &value) override;

	void add_child(shared<Control> child, int x, int y) override;
	void remove_child(Control *child) override;
	void add_page(const string &str);

	int cur_page;

	Array<Control*> pages; // sorted...
	Array<GtkWidget*> boxes; // sorted...
};

};

#endif /* CONTROLTABCONTROL_H_ */
