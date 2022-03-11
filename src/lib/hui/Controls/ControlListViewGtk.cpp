/*
 * ControlListView.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlListView.h"
#include "../hui.h"


#ifdef HUI_API_GTK

namespace hui
{

void *get_gtk_image_pixbuf(const string &image); // -> hui_menu_gtk.cpp
string tree_get_cell(GtkTreeModel *store, GtkTreeIter &iter, int column);

void list_toggle_callback(GtkCellRendererToggle *cell, gchar *path_string, gpointer data) {
	ControlListView *c = reinterpret_cast<ControlListView*>(data);
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


void list_edited_callback(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data) {
	ControlListView *c = reinterpret_cast<ControlListView*>(data);
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

string make_format_string_useful(const string &_format, int size) {
	string format_string = _format;
	for (int i=format_string.num; i<size; i++)
		format_string.add('t');
	format_string.resize(size);
	return format_string;
}

Array<GType> CreateTypeList(const string &_format, int size) {
	Array<GType> types;
	string format_string = make_format_string_useful(_format, size);

	for (char f: format_string) {
		if ((f == 'c') or (f == 'C'))
			types.add(G_TYPE_BOOLEAN);
		else if (f == 'i')
			types.add(GDK_TYPE_PIXBUF);
		else if (f == 'L')
			types.add(G_TYPE_INT);
		else
			types.add(G_TYPE_STRING);
	}
	return types;
}

void configure_tree_view_columns(Control *c, GtkWidget *view, const string &_format, const Array<string> &parts) {
	string format_string = make_format_string_useful(_format, parts.num);

	foreachi (char f, format_string, i){
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;
		if (f == 'C') {
			// editable checkbox
   			renderer = gtk_cell_renderer_toggle_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(parts[i]), renderer, "active", i, nullptr);
			g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(i));
			g_signal_connect (G_OBJECT(renderer), "toggled", G_CALLBACK(list_toggle_callback), c);
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
			g_signal_connect(renderer, "edited", G_CALLBACK(list_edited_callback), c);
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

void on_gtk_list_activate(GtkWidget *widget, void* a, void* b, gpointer data)
{	reinterpret_cast<Control*>(data)->notify(EventID::ACTIVATE);	}

void on_gtk_list_select(GtkTreeSelection *selection, gpointer data)
{	reinterpret_cast<Control*>(data)->notify(EventID::SELECT, false);	}


#if !GTK_CHECK_VERSION(4,0,0)
gboolean OnGtkListButton(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	if (event->button != GDK_BUTTON_SECONDARY) // right
		return false;
	if (event->type != GDK_BUTTON_PRESS)
		return false;
	auto *c = reinterpret_cast<ControlListView*>(user_data);
	if (event->window != gtk_tree_view_get_bin_window(GTK_TREE_VIEW(c->widget)))
		return false;
	c->on_click(event->x, event->y);
	return false;
}
#endif

void ControlListView::on_click(double x, double y) {
	panel->win->input.column = -1;
	panel->win->input.row = -1;
	GtkTreePath *path = nullptr;
	int cell_x = 0, cell_y = 0;
	panel->win->input.x = x;
	panel->win->input.y = y;
#if GTK_CHECK_VERSION(4,0,0)
	// remove frame/bar
	int tx, ty;
	gtk_tree_view_convert_widget_to_bin_window_coords(GTK_TREE_VIEW(widget), x, y, &tx, &ty);
	x = tx;
	y = ty;
#else
	int tx = x, ty = y;
#endif
	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), tx, ty, &path, nullptr, &cell_x, &cell_y)) {
		gint *indices = gtk_tree_path_get_indices(path);
		panel->win->input.row = indices[0];
		gtk_tree_path_free(path);
	}
	notify(EventID::RIGHT_BUTTON_DOWN, false);
}


void OnGtkListRowDeleted(GtkTreeModel *tree_model, GtkTreePath *path, gpointer user_data) {
	auto *lv = reinterpret_cast<ControlListView*>(user_data);
	if (!lv->allow_change_messages)
		return;

	//msg_write("row del");
	gint *indices = gtk_tree_path_get_indices(path);

	if (indices[0] >= lv->row_target) {
		lv->panel->win->input.row = indices[0] - 1;
		lv->panel->win->input.row_target = lv->row_target;
	} else {
		lv->panel->win->input.row = indices[0];
		lv->panel->win->input.row_target = lv->row_target - 1;
	}
	lv->notify(EventID::MOVE, false);
}

void OnGtkListRowInserted(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {
	auto *lv = reinterpret_cast<ControlListView*>(user_data);
	if (!lv->allow_change_messages)
		return;
	//msg_write("row insert");
	gint *indices = gtk_tree_path_get_indices(path);
	lv->row_target = indices[0];
}



#if GTK_CHECK_VERSION(4,0,0)
static void on_gtk_list_gesture_click_pressed(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<ControlListView*>(user_data);
	//win_set_mouse_pos(c->panel->win, (float)x, (float)y);
	c->on_click(x, y);
}
#endif


ControlListView::ControlListView(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_LISTVIEW, id)
{
	auto parts = split_title(title);

#if GTK_CHECK_VERSION(4,0,0)
	GtkWidget *sw = gtk_scrolled_window_new();
#else
	GtkWidget *sw = gtk_scrolled_window_new(nullptr, nullptr);
#endif
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	string fmt = option_value(get_option_from_title(title), "format");

	// "model"
	auto types = CreateTypeList(fmt, parts.num);
	auto store = gtk_list_store_newv(types.num, &types[0]);

	// "view"
	auto view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
//	g_object_unref(G_OBJECT(store));
	g_signal_connect(G_OBJECT(view), "row-activated", G_CALLBACK(&on_gtk_list_activate), this);

	// capture right click
#if GTK_CHECK_VERSION(4,0,0)
	auto gesture_click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture_click), GDK_BUTTON_SECONDARY);
	gtk_widget_add_controller(view, GTK_EVENT_CONTROLLER(gesture_click));
	g_signal_connect(G_OBJECT(gesture_click), "pressed", G_CALLBACK(&on_gtk_list_gesture_click_pressed), this);
#else
	g_signal_connect(G_OBJECT(view), "button-press-event", G_CALLBACK(&OnGtkListButton), this);
	gtk_widget_add_events(view, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
#endif

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(&on_gtk_list_select), this);


	// drag'n'drop reordering
	g_signal_connect(G_OBJECT(store), "row-inserted", G_CALLBACK(&OnGtkListRowInserted), this);
	g_signal_connect(G_OBJECT(store), "row-deleted", G_CALLBACK(&OnGtkListRowDeleted), this);
	allow_change_messages = true;

	row_target = -1;

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

	configure_tree_view_columns(this, view, fmt, parts);
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	set_options(get_option_from_title(title));
}

string ControlListView::get_string() {
	return "";
}

void ControlListView::__set_string(const string &str) {
	__add_string(str);
}

void set_list_cell(GtkListStore *store, GtkTreeIter &iter, int column, const string &str) {
	GType type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(store), column);
	if (type == G_TYPE_STRING) {
		gtk_list_store_set(store, &iter, column, sys_str(str), -1);
	} else if (type == G_TYPE_BOOLEAN) {
		gtk_list_store_set(store, &iter, column, (str == "1") or (str == "true"), -1);
	} else if (type == GDK_TYPE_PIXBUF) {
		GdkPixbuf *p = (GdkPixbuf*)get_gtk_image_pixbuf(str);
		if (p)
			gtk_list_store_set(store, &iter, column, p, -1);
	}
}

void ControlListView::__add_string(const string& str) {
	allow_change_messages = false;
	GtkTreeIter iter;
	auto parts = split_title(str);
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_list_store_append(store, &iter);
	for (int j=0; j<parts.num; j++)
		set_list_cell(store, iter, j, parts[j]);
	allow_change_messages = true;
}

void ControlListView::__set_int(int i) {
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (i >= 0) {
		GtkTreeIter iter;
		if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, i)) {
			gtk_tree_selection_select_iter(sel, &iter);
			GtkTreePath *path = gtk_tree_path_new_from_indices(i, -1);
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(widget), path, nullptr, false, 0, 0);
			gtk_tree_path_free(path);
		}
	} else {
		gtk_tree_selection_unselect_all(sel);
	}
}

int ControlListView::get_int() {
	auto sel = get_selection();
	if (sel.num > 0)
		return sel[0];
	return -1;
}

void ControlListView::__set_float(float f) {
	//gtk_tree_view_scroll_to_point(GTK_TREE_VIEW(widget), 0, f);
	//auto v = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(widget));
	//gtk_adjustment_set_value(GTK_ADJUSTMENT(v), f);
}

float ControlListView::get_float() {
	auto v = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(widget));
	return gtk_adjustment_get_value(GTK_ADJUSTMENT(v));
}

void ControlListView::__change_string(int row, const string& str) {
	if (row < 0)
		return;
	auto parts = split_title(str);
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		for (int j=0; j<parts.num; j++)
			set_list_cell(store, iter, j, parts[j]);
}

void ControlListView::__remove_string(int row) {
	if (row < 0)
		return;
	allow_change_messages = false;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		gtk_list_store_remove(store, &iter);
	allow_change_messages = true;
}

string ControlListView::get_cell(int row, int column) {
	if (row < 0)
		return "";
	GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;
	if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		return "";
	return tree_get_cell(store, iter, column);
}

void ControlListView::__set_cell(int row, int column, const string& str) {
	if (row < 0)
		return;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		set_list_cell(store, iter, column, str);
}

Array<int> ControlListView::get_selection() {
	Array<int> selected;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));

	for (int i=0; ; i++) {
		GtkTreeIter iter;
		if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, i))
			return selected;
		if (gtk_tree_selection_iter_is_selected(sel, &iter))
			selected.add(i);

	}
	return selected;
}

void ControlListView::__set_selection(const Array<int> &selected) {
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_unselect_all(sel);
	for (int i: selected) {
		GtkTreeIter iter;
		if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, i))
			continue;
		gtk_tree_selection_select_iter(sel, &iter);

	}
}

void ControlListView::__reset() {
	allow_change_messages = false;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_list_store_clear(store);
	allow_change_messages = true;
}

void ControlListView::__set_option(const string &op, const string &value) {
	if ((op == "multiline") or (op == "selectmulti")) {
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	} else if (op == "selectsingle") {
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
	} else if (op == "nobar") {
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), false);
	} else if (op == "bar") {
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), value._bool());
	} else if (op == "reorderable") {
		gtk_tree_view_set_reorderable(GTK_TREE_VIEW(widget), true);
	} else if (op == "singleclickactivate") {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(widget), true);
#else
		msg_error("ListView.singleclickactivate gtk3...");
#endif
	}
}

};

#endif
