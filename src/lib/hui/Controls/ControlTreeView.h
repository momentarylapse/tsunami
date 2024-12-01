/*
 * ControlTreeView.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef CONTROLTREEVIEW_H_
#define CONTROLTREEVIEW_H_

#include "Control.h"

#include <gtk/gtk.h>

namespace hui {

class ControlTreeView : public Control {
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
	bool is_expanded(int row) override;
	void __set_option(const string &op, const string &value) override;

	bool allow_change_messages = true;
	int row_target;

	string effective_format;
#if GTK_CHECK_VERSION(4,0,0)
	GListStore *store;
	GtkTreeListModel *tree_model;
	GtkSelectionModel *selection_model;
	bool is_list_view = false;
	bool is_column_view = false;
	bool is_grid_view = false;
	Array<GtkColumnViewColumn*> columns;
	Array<GtkListItemFactory*> factories;

	struct ItemMapper {
		ControlListView *list_view;
		GtkWidget *widget, *parent;
		GtkListItem *item;
		int column;
		int row_in_model;
	};
	owned_array<ItemMapper> _item_map_;
	int hover = -1;
#else
	Array<GtkTreeIter> _item_;
#endif

};

};

#endif /* CONTROLTREEVIEW_H_ */
