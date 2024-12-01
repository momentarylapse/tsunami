/*
 * ControlTreeView.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "../hui.h"
#include "ControlTreeView.h"
#include "ControlListView.h"
#include "../../base/iter.h"
#include "../../os/msg.h"

#ifdef HUI_API_GTK

#include <gtk/gtk.h>

namespace hui
{

#define dbo(s) //msg_write(s)

string make_format_string_useful(const string &_format, int size);


#if GTK_CHECK_VERSION(4,0,0)
void on_gtk_list_activate(GtkWidget* widget, guint pos, ControlListView* self);
void on_gtk_list_gesture_click_pressed(GtkGestureClick *gesture, int n_press, double x, double y, ControlListView *c);
void gtk_list_item_widget_enter_cb(GtkEventControllerMotion *controller, double x, double y, ControlListView::ItemMapper *h);
void gtk_list_item_widget_leave_cb(GtkEventControllerMotion *controller, double x, double y, ControlListView::ItemMapper *h);
//ControlListView::ItemMapper *list_view_find_item(ControlListView *lv, GtkListItem *list_item);
void list_view_notify_cell_change(GtkWidget *widget, ControlListView::ItemMapper *h, const string &val);
void on_gtk_list_checkbox_clicked(GtkWidget *widget, ControlListView::ItemMapper *h);
void on_gtk_list_edit_changed(GtkWidget *widget, ControlListView::ItemMapper *h);
void on_gtk_list_item_setup(GtkListItemFactory *factory, GtkListItem *list_item, void *user_data);
void on_gtk_list_item_bind(GtkListItemFactory *factory, GtkListItem *list_item, ControlListView *list_view);
void on_gtk_column_view_activate(GtkColumnView* self, guint position, gpointer user_data);
void on_gtk_selection_model_selection_changed(GtkSelectionModel* self, guint position, guint n_items, gpointer user_data);
#endif

#if !GTK_CHECK_VERSION(4,0,0)
Array<GType> CreateTypeList(const string &_format);


void tree_toggle_callback(GtkCellRendererToggle *cell, gchar *path_string, gpointer data) {
	dbo("TREE TOGGLE");
	auto *c = reinterpret_cast<Control*>(data);
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));
	gtk_tree_model_get_iter(model, &iter, path);
	bool state = (bool)gtk_cell_renderer_toggle_get_active(cell);
	state = !state;
	if (c->type == CONTROL_LISTVIEW)
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, state, -1);
	else if (c->type == CONTROL_TREEVIEW)
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, column, state, -1);

	c->panel->win->input.column = column;
	c->panel->win->input.row = s2i(path_string);
	c->notify(EventID::CHANGE, false);
	gtk_tree_path_free(path);
}


void tree_edited_callback(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data) {
	dbo("TREE EDITED");
	auto *c = reinterpret_cast<Control*>(data);
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));
	gtk_tree_model_get_iter(model, &iter, path);
	if (c->type == CONTROL_LISTVIEW)
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, new_text, -1);
	else if (c->type == CONTROL_TREEVIEW)
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, column, new_text, -1);


	c->panel->win->input.column = column;
	c->panel->win->input.row = s2i(path_string);
	c->notify(EventID::CHANGE, false);
	gtk_tree_path_free(path);
}

void configure_tree_view_columns(Control *c, GtkWidget *view, const string &_format, const Array<string> &parts) {
	dbo("CONFIGURE TREE");
	string format_string = make_format_string_useful(_format, parts.num);

	for (auto&& [i,f]: enumerate(format_string)) {
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;
		if (f == 'C' or f == 'S') {
			// editable checkbox
   			renderer = gtk_cell_renderer_toggle_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "active", i, nullptr);
			g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(i));
			g_signal_connect (G_OBJECT(renderer), "toggled", G_CALLBACK(tree_toggle_callback), c);
		} else if (f == 'c') {
			// constant checkbox
   			renderer = gtk_cell_renderer_toggle_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "active", i, nullptr);
		} else if (f == 'i') {
			// image
   			renderer = gtk_cell_renderer_pixbuf_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "pixbuf", i, nullptr);
		} else if (f == 'L') {
			// list
			renderer = gtk_cell_renderer_combo_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "active", i, nullptr);
		} else if (f == 'T') {
			// editable text
			renderer = gtk_cell_renderer_text_new();
			g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(i));
			g_object_set(renderer, "editable", TRUE, nullptr);
			g_signal_connect(renderer, "edited", G_CALLBACK(tree_edited_callback), c);
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "text", i, nullptr);
		} else if (f == 'm'){
			// constant text with markup
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "markup", i, nullptr);
		} else {
			// constant text
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "text", i, nullptr);
		}
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	}
}


void on_gtk_tree_activate(GtkWidget *widget, void* a, void* b, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::ACTIVATE);
}

void on_gtk_tree_select(GtkTreeSelection *selection, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::SELECT, false);
}

void *get_gtk_image_paintable(const string &image); // -> hui_menu_gtk.cpp

#endif

ControlTreeView::ControlTreeView(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_TREEVIEW, id)
{
	auto parts = split_title(title);
	auto options = get_option_from_title(title);
	string fmt = option_value(options, "format");


#if GTK_CHECK_VERSION(4,0,0)
	GtkWidget *sw = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#else
	GtkWidget *sw = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

#endif


#if GTK_CHECK_VERSION(4,0,0)
	effective_format = make_format_string_useful(fmt, parts.num);
	msg_write(effective_format);

	// "model"
	store = g_list_store_new(G_TYPE_OBJECT);

//	tree_model = gtk_tree_list_model_new(store, false, false, &on_gtk_tree_list_model_create_model, this, nullptr);

	auto sel = gtk_single_selection_new(G_LIST_MODEL(tree_model));
	gtk_single_selection_set_autoselect(sel, false);
	gtk_single_selection_set_can_unselect(sel, true);
	selection_model = GTK_SELECTION_MODEL(sel);

	GtkWidget *view;
	if (parts.num > 1) {
		is_column_view = true;
		view = gtk_column_view_new(GTK_SELECTION_MODEL(selection_model));

		for (auto &p: parts) {
			auto factory = gtk_signal_list_item_factory_new();
			factories.add(factory);
			g_signal_connect(factory, "setup", G_CALLBACK(on_gtk_list_item_setup), this);
			g_signal_connect(factory, "bind", G_CALLBACK(on_gtk_list_item_bind), this);

			auto col = gtk_column_view_column_new(p.c_str(), factory);
			columns.add(col);
			gtk_column_view_append_column(GTK_COLUMN_VIEW(view), GTK_COLUMN_VIEW_COLUMN(col));
		}
		if (columns.num >= 2 and fmt.back() == 'B')
			gtk_column_view_column_set_expand(GTK_COLUMN_VIEW_COLUMN(columns[columns.num - 2]), true);
		else
			gtk_column_view_column_set_expand(GTK_COLUMN_VIEW_COLUMN(columns.back()), true);

	} else {
		auto factory = gtk_signal_list_item_factory_new();
		factories.add(factory);
		g_signal_connect(factory, "setup", G_CALLBACK(on_gtk_list_item_setup), this);
		g_signal_connect(factory, "bind", G_CALLBACK(on_gtk_list_item_bind), this);

		if (option_has(options, "grid")) {
			is_grid_view = true;
			view = gtk_grid_view_new(GTK_SELECTION_MODEL(selection_model), factory);
			gtk_orientable_set_orientation(GTK_ORIENTABLE(view), GTK_ORIENTATION_HORIZONTAL);
		} else {
			is_list_view = true;
			view = gtk_list_view_new(GTK_SELECTION_MODEL(selection_model), factory);
			//gtk_list_view_set_show_separators(GTK_LIST_VIEW(view), true);
			//gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(view), true);
			//gtk_list_view_set_enable_rubberband(GTK_LIST_VIEW(view), true);
		}
	}



	g_signal_connect(G_OBJECT(selection_model), "selection-changed", G_CALLBACK(&on_gtk_selection_model_selection_changed), this);
	g_signal_connect(G_OBJECT(view), "activate", G_CALLBACK(&on_gtk_list_activate), this);

	// capture right click
	auto gesture_click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture_click), GDK_BUTTON_SECONDARY);
	gtk_widget_add_controller(view, GTK_EVENT_CONTROLLER(gesture_click));
	g_signal_connect(G_OBJECT(gesture_click), "pressed", G_CALLBACK(&on_gtk_list_gesture_click_pressed), this);



#else
	// "model"
	Array<GType> types = CreateTypeList(make_format_string_useful(fmt, parts.num));
	GtkTreeStore *store = gtk_tree_store_newv(types.num, &types[0]);

	// "view"
	GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	g_signal_connect(G_OBJECT(view),"row-activated",G_CALLBACK(&on_gtk_tree_activate),this);

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(&on_gtk_tree_select), this);
#endif

	// frame
	frame = sw;
	if (panel->border_width > 0) {
		frame = gtk_frame_new(nullptr);
#if GTK_CHECK_VERSION(4,0,0)
		gtk_frame_set_child(GTK_FRAME(frame), sw);
#else
		gtk_container_add(GTK_CONTAINER(frame), sw);
#endif
	}

#if GTK_CHECK_VERSION(4,0,0)
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), view);
#else
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);
#endif

	widget = view;
	take_gtk_ownership();

#if GTK_CHECK_VERSION(4,0,0)
#else
	configure_tree_view_columns(this, view, fmt, parts);
#endif
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	focusable = true;
	set_options(get_option_from_title(title));
}

string ControlTreeView::get_string() {
	return "";
}

void ControlTreeView::__set_string(const string &str) {
	__add_string(str);
}

#if !GTK_CHECK_VERSION(4,0,0)
void set_tree_cell(GtkTreeStore *store, GtkTreeIter &_iter, int column, const string &str) {
	GtkTreeIter iter = _iter;
	GType type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(store), column);
	if (type == G_TYPE_STRING)
		gtk_tree_store_set(store, &iter, column, sys_str(str), -1);
	else if (type == G_TYPE_BOOLEAN)
		gtk_tree_store_set(store, &iter, column, (str == "1") || (str == "true"), -1);
	else if (type == GDK_TYPE_PIXBUF) {
		GdkPixbuf *p = (GdkPixbuf*)get_gtk_image_paintable(str);
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
#endif

void ControlTreeView::__add_string(const string& _str) {
	allow_change_messages = false;
	auto parts = split_title(_str);
#if GTK_CHECK_VERSION(4,0,0)
/*	msg_write("+++ " + str(parts));
	//auto ob = gtk_string_object_new(str.c_str());
	//g_list_store_append(G_LIST_STORE(store), ob);
	auto row = g_list_store_new(G_TYPE_OBJECT);
	for (auto &p: parts) {
		auto ob = gtk_string_object_new(p.c_str());
		g_list_store_append(G_LIST_STORE(row), ob);
	}
	g_list_store_append(G_LIST_STORE(store), row);*/
#else
	GtkTreeIter iter;
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_append(store, &iter, nullptr);
	for (int j=0; j<parts.num; j++)
		set_tree_cell(store, iter, j, parts[j]);
	_item_.add(iter);
#endif
	allow_change_messages = true;
}

void ControlTreeView::__set_int(int i) {
#if !GTK_CHECK_VERSION(4,0,0)
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (i >= 0) {
		gtk_tree_selection_select_iter(sel, &_item_[i]);
		GtkTreePath *path = gtk_tree_path_new_from_indices(i, -1);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(widget), path, nullptr, false, 0, 0);
		gtk_tree_path_free(path);
	} else {
		gtk_tree_selection_unselect_all(sel);
	}
#endif
}

int ControlTreeView::get_int() {
#if !GTK_CHECK_VERSION(4,0,0)
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	for (int j=0;j<_item_.num;j++)
		if (gtk_tree_selection_iter_is_selected(sel, &_item_[j]))
			return j;
#endif
	return -1;
}

void ControlTreeView::__add_child_string(int parent_row, const string& str) {
#if !GTK_CHECK_VERSION(4,0,0)
	GtkTreeIter iter;
	auto parts = split_title(str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_append(store, &iter, &_item_[parent_row]);
	for (int j=0;j<parts.num;j++)
		set_tree_cell(store, iter, j, parts[j]);
	_item_.add(iter);
#endif
}

void ControlTreeView::__change_string(int row, const string& str) {
#if !GTK_CHECK_VERSION(4,0,0)
	auto parts = split_title(str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	/*for (int j=0;j<PartString.num;j++)
		gtk_tree_store_set(store, &c->_item_[row], j, sys_str(PartString[j]), -1);*/
	if (gtk_tree_store_iter_is_valid(store, &_item_[row]))
		for (int j=0;j<parts.num;j++)
			set_tree_cell(store, _item_[row], j, parts[j]);
#endif
}

void ControlTreeView::__remove_string(int row) {
#if !GTK_CHECK_VERSION(4,0,0)
	if ((row < 0) or (row >= _item_.num))
		return;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	if (gtk_list_store_iter_is_valid(store, &_item_[row])){
		gtk_list_store_remove(store, &_item_[row]);
		_item_.erase(row);
	}
#endif
}

string ControlTreeView::get_cell(int row, int column) {
#if GTK_CHECK_VERSION(4,0,0)
	return "";
#else
	if ((row < 0) or (row >= _item_.num))
		return "";
	GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	return tree_get_cell(store, _item_[row], column);
#endif
}

void ControlTreeView::__set_cell(int row, int column, const string& str) {
#if !GTK_CHECK_VERSION(4,0,0)
	if ((row < 0) or (row >= _item_.num))
		return;
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	if (gtk_tree_store_iter_is_valid(store, &_item_[row]))
		set_tree_cell(store, _item_[row], column, str);
#endif
}

Array<int> ControlTreeView::get_selection() {
	Array<int> sel;
#if !GTK_CHECK_VERSION(4,0,0)
	GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
	for (int j=0;j<_item_.num;j++)
		if (gtk_tree_selection_iter_is_selected(s, &_item_[j])) {
			sel.add(j);
		}
#endif
	return sel;
}

void ControlTreeView::__set_selection(const Array<int>& sel) {
#if !GTK_CHECK_VERSION(4,0,0)
	GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_unselect_all(s);
	for (int j=0;j<sel.num;j++)
		gtk_tree_selection_select_iter(s, &_item_[sel[j]]);
#endif
}

void ControlTreeView::__reset() {
#if !GTK_CHECK_VERSION(4,0,0)
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_clear(store);
	_item_.clear();
#endif
}

void ControlTreeView::expand(int row, bool expand) {
#if !GTK_CHECK_VERSION(4,0,0)
	if (row < 0) {
		// expand all
		if (expand)
			gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));
		else
			gtk_tree_view_collapse_all(GTK_TREE_VIEW(widget));
	}
#endif
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
#if !GTK_CHECK_VERSION(4,0,0)
	if (op == "nobar")
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), false);
	else if (op == "bar")
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), val_is_positive(value, true));
#endif
}

};

#endif
