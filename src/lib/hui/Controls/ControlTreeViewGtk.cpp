/*
 * ControlTreeView.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "../hui.h"
#include "ControlTreeView.h"

#ifdef HUI_API_GTK

namespace hui
{

void list_toggle_callback(GtkCellRendererToggle *cell, gchar *path_string, gpointer data);
void list_edited_callback(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data);
Array<GType> CreateTypeList(const string &_format, int size);
void configure_tree_view_columns(Control *c, GtkWidget *view, const string &_format, const Array<string> &parts);
void on_gtk_list_activate(GtkWidget *widget, void* a, void* b, gpointer data);
void on_gtk_list_select(GtkTreeSelection *selection, gpointer data);

void *get_gtk_image_pixbuf(const string &image); // -> hui_menu_gtk.cpp

ControlTreeView::ControlTreeView(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_TREEVIEW, id)
{
	auto parts = split_title(title);
	string fmt = option_value(get_option_from_title(title), "format");

	GtkWidget *sw = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// "model"
	Array<GType> types = CreateTypeList(fmt, parts.num);
	GtkTreeStore *store = gtk_tree_store_newv(types.num, &types[0]);

	// "view"
	GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	g_signal_connect(G_OBJECT(view),"row-activated",G_CALLBACK(&on_gtk_list_activate),this);

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(&on_gtk_list_select), this);

	// frame
	frame = sw;
	if (panel->border_width > 0) {
		frame = gtk_frame_new(nullptr);
		gtk_container_add(GTK_CONTAINER(frame), sw);
	}
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);

	widget = view;

	configure_tree_view_columns(this, view, fmt, parts);
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	set_options(get_option_from_title(title));
}

string ControlTreeView::get_string() {
	return "";
}

void ControlTreeView::__set_string(const string &str) {
	__add_string(str);
}

void set_tree_cell(GtkTreeStore *store, GtkTreeIter &_iter, int column, const string &str) {
	GtkTreeIter iter = _iter;
	GType type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(store), column);
	if (type == G_TYPE_STRING)
		gtk_tree_store_set(store, &iter, column, sys_str(str), -1);
	else if (type == G_TYPE_BOOLEAN)
		gtk_tree_store_set(store, &iter, column, (str == "1") || (str == "true"), -1);
	else if (type == GDK_TYPE_PIXBUF) {
		GdkPixbuf *p = (GdkPixbuf*)get_gtk_image_pixbuf(str);
		if (p)
			gtk_tree_store_set(store, &iter, column, p, -1);
	}
}

string tree_get_cell(GtkTreeModel *store, GtkTreeIter &_iter, int column) {
	GtkTreeIter iter = _iter;
	string r;
	GType type = gtk_tree_model_get_column_type(store, column);
	if (type == G_TYPE_STRING) {
		gchar *str;
		gtk_tree_model_get(store, &iter, column, &str, -1);
		r = str;
		g_free (str);
	} else if (type == G_TYPE_BOOLEAN) {
		bool state;
		gtk_tree_model_get(store, &iter, column, &state, -1);
		r = b2s(state);
	}
	return r;
}

void ControlTreeView::__add_string(const string& str) {
	GtkTreeIter iter;
	auto parts = split_title(str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_append(store, &iter, nullptr);
	for (int j=0; j<parts.num; j++)
		set_tree_cell(store, iter, j, parts[j]);
	_item_.add(iter);
}

void ControlTreeView::__set_int(int i) {
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (i >= 0) {
		gtk_tree_selection_select_iter(sel, &_item_[i]);
		GtkTreePath *path = gtk_tree_path_new_from_indices(i, -1);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(widget), path, nullptr, false, 0, 0);
		gtk_tree_path_free(path);
	} else {
		gtk_tree_selection_unselect_all(sel);
	}
}

int ControlTreeView::get_int() {
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	for (int j=0;j<_item_.num;j++)
		if (gtk_tree_selection_iter_is_selected(sel, &_item_[j]))
			return j;
	return -1;
}

void ControlTreeView::__add_child_string(int parent_row, const string& str) {
	GtkTreeIter iter;
	auto parts = split_title(str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_append(store, &iter, &_item_[parent_row]);
	for (int j=0;j<parts.num;j++)
		set_tree_cell(store, iter, j, parts[j]);
	_item_.add(iter);
}

void ControlTreeView::__change_string(int row, const string& str) {
	auto parts = split_title(str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	/*for (int j=0;j<PartString.num;j++)
		gtk_tree_store_set(store, &c->_item_[row], j, sys_str(PartString[j]), -1);*/
	if (gtk_tree_store_iter_is_valid(store, &_item_[row]))
		for (int j=0;j<parts.num;j++)
			set_tree_cell(store, _item_[row], j, parts[j]);
}

void ControlTreeView::__remove_string(int row) {
	if ((row < 0) or (row >= _item_.num))
		return;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	if (gtk_list_store_iter_is_valid(store, &_item_[row])){
		gtk_list_store_remove(store, &_item_[row]);
		_item_.erase(row);
	}
}

string ControlTreeView::get_cell(int row, int column) {
	if ((row < 0) or (row >= _item_.num))
		return "";
	GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	return tree_get_cell(store, _item_[row], column);
}

void ControlTreeView::__set_cell(int row, int column, const string& str) {
	if ((row < 0) or (row >= _item_.num))
		return;
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	if (gtk_tree_store_iter_is_valid(store, &_item_[row]))
		set_tree_cell(store, _item_[row], column, str);
}

Array<int> ControlTreeView::get_selection() {
	Array<int> sel;
	GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
	for (int j=0;j<_item_.num;j++)
		if (gtk_tree_selection_iter_is_selected(s, &_item_[j])) {
			sel.add(j);
		}
	return sel;
}

void ControlTreeView::__set_selection(const Array<int>& sel) {
	GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_unselect_all(s);
	for (int j=0;j<sel.num;j++)
		gtk_tree_selection_select_iter(s, &_item_[sel[j]]);
}

void ControlTreeView::__reset() {
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_clear(store);
	_item_.clear();
}

void ControlTreeView::expand(int row, bool expand) {
}

void ControlTreeView::expand_all(bool expand) {
	if (expand)
		gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));
	else
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(widget));
}

bool ControlTreeView::is_expanded(int row) {
	/*HuiControl *c = _GetControl_(_id);
	if (c->type == HuiKindTreeView){
		if (expand)
			gtk_tree_view_expand_row(GTK_TREE_VIEW(c->widget), c->_item_[row]);
		else
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(c->widget));
	}*/
	return false;
}

void ControlTreeView::__set_option(const string &op, const string &value) {
	if (op == "nobar")
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), false);
	else if (op == "bar")
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), val_is_positive(value, true));
}

};

#endif
