/*
 * ControlListView.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLLISTVIEW_H_
#define CONTROLLISTVIEW_H_

#include "Control.h"

namespace hui
{

class ControlListView : public Control
{
public:
	ControlListView(const string &text, const string &id, Panel *panel);
	string get_string() override;
	void __set_string(const string &str) override;
	void __add_string(const string &str) override;
	void __set_int(int i) override;
	int get_int() override;
	void __change_string(int row, const string &str) override;
	void __remove_string(int row) override;
	string get_cell(int row, int column) override;
	void __set_cell(int row, int column, const string &str) override;
	Array<int> get_selection() override;
	void __set_selection(const Array<int> &sel) override;
	void __reset() override;
	void __set_option(const string &op, const string &value) override;

	bool allow_change_messages;
	int row_target;
};

};

#endif /* CONTROLLISTVIEW_H_ */
