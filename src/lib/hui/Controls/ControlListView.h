/*
 * ControlListView.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLLISTVIEW_H_
#define CONTROLLISTVIEW_H_

#include "Control.h"

#include <gtk/gtk.h>

namespace hui
{

class ControlListView : public Control {
public:
	ControlListView(const string &text, const string &id, Panel *panel);
	string get_string() override;
	void __set_string(const string &str) override;
	void __add_string(const string &str) override;
	void __set_int(int i) override;
	int get_int() override;
	void __set_float(float f) override;
	float get_float() override;
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
	bool reorderable = false;

	string effective_format;
#if GTK_CHECK_VERSION(4,0,0)
	GListStore *store;
	GtkSelectionModel *selection_model;
	bool is_list_view = false;
	bool is_column_view = false;
	bool is_grid_view = false;
	Array<GtkColumnViewColumn*> columns;
	Array<GtkListItemFactory*> factories;
	GtkWidget *overlay_drawing_area = nullptr;

	void __update_cell(int row, int column, const string& str, bool update_view);

	int potential_drop_row = -1;

	struct ItemMapper {
		ControlListView *list_view;
		GtkWidget *widget, *parent;
		GtkListItem *item;
		int column;
		int row_in_model;
	};
	owned_array<ItemMapper> _item_map_;
	int hover = -1;
#endif

	void on_right_click(double x, double y);
};

};

#endif /* CONTROLLISTVIEW_H_ */
