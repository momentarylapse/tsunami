/*
 * ControlTreeView.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef CONTROLTREEVIEW_H_
#define CONTROLTREEVIEW_H_

#include "Control.h"

namespace hui
{

class ControlTreeView : public Control
{
public:
	ControlTreeView(const string &title, const string &id, Panel *panel);
	string get_string() override;
	void __set_string(const string &str) override;
	void __add_string(const string &str) override;
	void __set_int(int i) override;
	int get_int() override;
	void __add_child_string(int parent_row, const string &str) override;
	void __change_string(int row, const string &str) override;
	void __remove_string(int row) override;
	string get_cell(int row, int column) override;
	void __set_cell(int row, int column, const string &str) override;
	Array<int> get_selection() override;
	void __set_selection(const Array<int> &sel) override;
	void __reset() override;
	void expand(int row, bool expand) override;
	void expand_all(bool expand) override;
	bool is_expanded(int row) override;
	void __set_option(const string &op, const string &value) override;

#ifdef HUI_API_GTK
	Array<GtkTreeIter> _item_;
#endif
};

};

#endif /* CONTROLTREEVIEW_H_ */
