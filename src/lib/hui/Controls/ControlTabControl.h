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

class ControlTabControl : public Control
{
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
	void __set_option(const string &op, const string &value);

	void add(Control *child, int x, int y) override;
	void addPage(const string &str);

	int cur_page;

	Array<Control*> pages; // sorted...
};

};

#endif /* CONTROLTABCONTROL_H_ */
