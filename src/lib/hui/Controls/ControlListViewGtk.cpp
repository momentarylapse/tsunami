/*
 * ControlListView.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlListView.h"
#include "../hui.h"
#include "../internal.h"
#include "../../base/iter.h"
#include "../../base/algo.h"
#include "../../os/msg.h"


#ifdef HUI_API_GTK

namespace hui
{

#define dbo(s) //msg_write(s)

string tree_get_cell(GtkTreeModel *store, GtkTreeIter &iter, int column);
void configure_tree_view_columns(Control *c, GtkWidget *view, const string &_format, const Array<string> &parts);


string make_format_string_useful(const string &_format, int size) {
	string format_string = _format;
	for (int i=format_string.num; i<size; i++)
		format_string.add('t');
	format_string.resize(size);
	return format_string;
}

Array<GType> CreateTypeList(const string &format_string) {
	Array<GType> types;
	for (char f: format_string) {
		if ((f == 'c') or (f == 'C') or (f == 'S'))
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

void on_gtk_list_activate(GtkWidget* widget, guint pos, ControlListView* self) {
	self->panel->win->input.row = pos;
	self->notify(EventID::ACTIVATE);
}

void on_gtk3_treeview_row_activated(GtkWidget* widget, void* a, void* b, ControlListView* self) {
	self->notify(EventID::ACTIVATE);
}

void ControlListView::on_right_click(double x, double y) {
	dbo("LIST ON RIGHT CLICK");
	panel->win->input.column = -1;
	panel->win->input.row = -1;
	panel->win->input.x = x;
	panel->win->input.y = y;
#if GTK_CHECK_VERSION(4,0,0)
	//msg_write(hover);
	panel->win->input.row = hover;
#else
	int cell_x = 0, cell_y = 0;
	int tx = x, ty = y;
	GtkTreePath *path = nullptr;
	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), tx, ty, &path, nullptr, &cell_x, &cell_y)) {
		gint *indices = gtk_tree_path_get_indices(path);
		panel->win->input.row = indices[0];
		gtk_tree_path_free(path);
	}
#endif
	notify(EventID::RIGHT_BUTTON_DOWN, false);
}



#if GTK_CHECK_VERSION(4,0,0)
void on_gtk_list_gesture_click_pressed(GtkGestureClick *gesture, int n_press, double x, double y, ControlListView *c) {
	dbo("...click " + i2s(c->hover));
	int hover = c->hover; // in case we mess something up

	// select hover (if not selected)
	auto sel = c->get_selection();
	if (sel.find(hover) < 0) {
		c->set_int(hover);
		c->notify(EventID::SELECT, false);
		hui::Application::do_single_main_loop();
	}

	c->hover = hover;
	c->on_right_click(x, y);
}

void gtk_list_item_widget_enter_cb(GtkEventControllerMotion *controller, double x, double y, ControlListView::ItemMapper *h) {
	h->list_view->hover = h->row_in_model;
	dbo("E " + i2s(h->row_in_model));
}

void gtk_list_item_widget_leave_cb(GtkEventControllerMotion *controller, ControlListView::ItemMapper *h) {
	h->list_view->hover = -1;
	dbo("L");
}

ControlListView::ItemMapper *list_view_find_item(ControlListView *lv, GtkListItem *list_item) {
	//base::find(list_view->_row_associators_, child))
	for (auto i: weak(lv->_item_map_))
		if (i->item == list_item)
			return i;
	return nullptr;
}

void on_list_view_cell_change(GtkWidget *widget, ControlListView::ItemMapper *h, const string &val) {
	int col = h->column;
	int row = h->row_in_model;
	h->list_view->__update_cell(row, col, val, false);
	if (!h->list_view->allow_change_messages)
		return;
	h->list_view->panel->win->input.column = col;
	h->list_view->panel->win->input.row = row;
	h->list_view->notify(EventID::CHANGE, false);
}

void on_gtk_list_checkbox_clicked(GtkWidget *widget, ControlListView::ItemMapper *h) {
	on_list_view_cell_change(widget, h, b2s(gtk_check_button_get_active(GTK_CHECK_BUTTON(widget))));
}

void on_gtk_list_edit_changed(GtkWidget *widget, ControlListView::ItemMapper *h) {
	on_list_view_cell_change(widget, h, gtk_editable_get_text(GTK_EDITABLE(widget)));
}


//----------------------------------------------------
// drag n drop

GdkContentProvider* on_drag_prepare(GtkDragSource *source, double x, double y, ControlListView::ItemMapper *m) {
	return gdk_content_provider_new_typed(G_TYPE_STRING, format("::::%s|%d", p2s(m->list_view), m->row_in_model).c_str());
}

void on_drag_begin(GtkDragSource *source, GdkDrag *drag, ControlListView::ItemMapper *m) {
	// Set the widget as the drag icon
	GdkPaintable *paintable = gtk_widget_paintable_new(gtk_widget_get_parent(m->parent));
	gtk_drag_source_set_icon(source, paintable, 0, 0);
	g_object_unref(paintable);
}

void on_gtk_list_overlay_draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer user_data) {
	auto lv = reinterpret_cast<ControlListView*>(user_data);
	if (lv->potential_drop_row < 0)
		return;
	float y = 0;
	if (lv->potential_drop_row > 0) {
		for (auto &i: lv->_item_map_)
			if (i->row_in_model == lv->potential_drop_row - 1) {
				graphene_rect_t bounds;
				if (!gtk_widget_compute_bounds(i->widget, lv->widget, &bounds))
					return;
				y = bounds.origin.y + bounds.size.height;
			}
	}

	cairo_set_source_rgba(cr, 1, 1, 1, 0.7f);
	cairo_rectangle(cr, 0, y-1, width, 3);
	cairo_fill(cr);
}

static gboolean on_gtk_list_drop(GtkDropTarget *target, const GValue *value, double x, double y, ControlListView* self) {
	gtk_widget_set_visible(self->overlay_drawing_area, false);
	if (G_VALUE_HOLDS(value, G_TYPE_STRING)) {
		string s = g_value_get_string(value);
		string m = "::::" + p2s(self) + "|";
		if (s.head(m.num) == m) {
			int row_source = s.sub(m.num)._int();
			self->panel->win->input.row = row_source;
			if (row_source >= self->potential_drop_row)
				self->panel->win->input.row_target = self->potential_drop_row;
			else
				self->panel->win->input.row_target = self->potential_drop_row - 1;
			if (self->panel->win->input.row != self->panel->win->input.row_target)
				self->notify(EventID::MOVE, false);
			return TRUE;
		}
	}
	return FALSE;
}

static GdkDragAction on_gtk_list_drop_motion(GtkDropTarget* target, gdouble x, gdouble y, ControlListView* self) {
	//msg_write(format("motion %f  %f", x, y));
	self->potential_drop_row = 0;
	for (auto &i: self->_item_map_) {
		graphene_rect_t bounds;
		if (gtk_widget_compute_bounds(i->widget, self->widget, &bounds)) {
			if (y >= bounds.origin.y + bounds.size.height / 2)
				self->potential_drop_row = i->row_in_model + 1;
		}
	}
	gtk_widget_set_visible(self->overlay_drawing_area, true);
	gtk_widget_queue_draw(self->overlay_drawing_area);
	return GDK_ACTION_COPY;
}

static void on_gtk_list_drop_leave(GtkDropTarget* target, ControlListView* self) {
	self->potential_drop_row = -1;
	gtk_widget_set_visible(self->overlay_drawing_area, false);
}

//----------------------------------------------------
// factory

void on_gtk_list_item_setup(GtkListItemFactory *factory, GtkListItem *list_item, void *user_data) {
	dbo("LIST SETUP " + p2s(list_item));
	auto list_view = reinterpret_cast<ControlListView*>(user_data);
	int col = base::find_index(list_view->factories, factory);
	if (col < 0 or col >= list_view->effective_format.num)
		return;

	// item mapping
	auto m = new ControlListView::ItemMapper{list_view, nullptr, nullptr, list_item, col, -1};
	list_view->_item_map_.add(m);

	// create widget
	char f = list_view->effective_format[col];
	GtkWidget *w;
	if (f == 'C') {
		w = gtk_check_button_new();
		g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(&on_gtk_list_checkbox_clicked), m);
	} else if (f == 'S') {
		w = gtk_switch_new();
	} else if (f == 'B') {
		w = gtk_button_new();
	} else if (f == 'i') {
		w = gtk_picture_new();
		gtk_picture_set_can_shrink(GTK_PICTURE(w), false);
		//gtk_picture_set_content_fit(GTK_PICTURE(w), false);
	} else if (f == 'T') {
		w = gtk_entry_new();
		gtk_entry_set_has_frame(GTK_ENTRY(w), false);
		g_signal_connect(G_OBJECT(w), "changed", G_CALLBACK(&on_gtk_list_edit_changed), m);
	} else {
		w = gtk_label_new("");
		//gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_LEFT);
		gtk_widget_set_halign(w, GTK_ALIGN_START);
	}
	gtk_list_item_set_child(list_item, w);
	m->widget = w;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void gtk_list_view_update_item_view(ControlListView* list_view, int column, GtkWidget* w, const char* s) {
	char f = list_view->effective_format[column];
	list_view->allow_change_messages = false;
	if (f == 'S') {
		gtk_switch_set_active(GTK_SWITCH(w), string(s)._bool());
	} else if (f == 'C') {
		gtk_check_button_set_active(GTK_CHECK_BUTTON(w), string(s)._bool());
	} else if (f == 'B') {
		//gtk_button_set_active(GTK_CHECK_BUTTON(w), string(s)._bool());
	} else if (f == 'i') {
		void* p = get_gtk_image_paintable(s);
#if GTK_CHECK_VERSION(4,0,0)
		gtk_picture_set_paintable(GTK_PICTURE(w), (GdkPaintable*)p);
#else
		// unused...
		gtk_picture_set_pixbuf(GTK_PICTURE(w), (GdkPixbuf*)p);
#endif
	} else if (f == 'T') {
		gtk_editable_set_text(GTK_EDITABLE(w), s);
	} else if (f == 'm') {
		gtk_label_set_markup(GTK_LABEL(w), s);
	} else {
		gtk_label_set_label(GTK_LABEL(w), s);
	}
	list_view->allow_change_messages = true;
}

void on_gtk_list_item_bind(GtkListItemFactory *factory, GtkListItem *list_item, ControlListView *list_view) {
	int column = base::find_index(list_view->factories, factory);
	if (column < 0 or column >= list_view->effective_format.num)
		return;
	auto row_model = gtk_list_item_get_item(list_item);
	if (!row_model)
		return;
	auto obj = g_list_model_get_item(G_LIST_MODEL(row_model), column);
	if (!obj)
		return;
	auto s = gtk_string_object_get_string(GTK_STRING_OBJECT(obj));
	int row_no = gtk_list_item_get_position(list_item);
	dbo(format("LIST BIND %s  %d %d", p2s(list_item), row_no, column));

	// update widget
	GtkWidget *w = gtk_list_item_get_child(list_item);
	gtk_list_view_update_item_view(list_view, column, w, s);

	// mapping stuff
	auto m = list_view_find_item(list_view, list_item);
	if (m) {
		m->row_in_model = row_no;
		if (!m->parent) {
			m->parent = gtk_widget_get_parent(w);
			//dbo("item parent " + p2s(m->parent));
			auto controller = gtk_event_controller_motion_new();
			g_signal_connect(controller, "enter", G_CALLBACK(gtk_list_item_widget_enter_cb), m);
			g_signal_connect(controller, "leave", G_CALLBACK(gtk_list_item_widget_leave_cb), m);
			gtk_widget_add_controller(m->parent, controller);

			if (list_view->reorderable) {
				GtkDragSource *drag_source = gtk_drag_source_new();
				g_signal_connect(drag_source, "prepare", G_CALLBACK(on_drag_prepare), m);
				g_signal_connect(drag_source, "drag-begin", G_CALLBACK(on_drag_begin), m);
				gtk_widget_add_controller(m->parent, GTK_EVENT_CONTROLLER(drag_source));
			}
		}
	} else {
		msg_error("hui.ListView.bind: no map");
	}
}

#pragma GCC diagnostic pop

void on_gtk_column_view_activate(GtkColumnView* self, guint position, gpointer user_data) {
	reinterpret_cast<Control*>(user_data)->notify(EventID::ACTIVATE);
}

void on_gtk_selection_model_selection_changed(GtkSelectionModel* self, guint position, guint n_items, gpointer user_data) {
	reinterpret_cast<Control*>(user_data)->notify(EventID::SELECT, false);
}

#else // gtk3

void on_gtk_list_select(GtkTreeSelection *selection, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::SELECT, false);
}

gboolean on_gtk_list_right_button(GtkWidget* widget, GdkEventButton* event, ControlListView* self) {
	dbo("LIST RIGHT BUTTON");
	if (event->button != GDK_BUTTON_SECONDARY) // right
		return false;
	if (event->type != GDK_BUTTON_PRESS)
		return false;
	if (event->window != gtk_tree_view_get_bin_window(GTK_TREE_VIEW(self->widget)))
		return false;
	self->on_right_click(event->x, event->y);
	return false;
}
void on_gtk_list_row_deleted(GtkTreeModel *tree_model, GtkTreePath *path, gpointer user_data) {
	dbo("ROW DEL");
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

void on_gtk_list_row_inserted(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {
	dbo("ROW INSERTED");
	auto *lv = reinterpret_cast<ControlListView*>(user_data);
	if (!lv->allow_change_messages)
		return;
	//msg_write("row insert");
	gint *indices = gtk_tree_path_get_indices(path);
	lv->row_target = indices[0];
}

#endif


ControlListView::ControlListView(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_LISTVIEW, id)
{
	auto parts = split_title(title);
	auto options = get_option_from_title(title);

#if GTK_CHECK_VERSION(4,0,0)
	GtkWidget *sw = gtk_scrolled_window_new();
#else
	GtkWidget *sw = gtk_scrolled_window_new(nullptr, nullptr);
#endif
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	string fmt = option_value(options, "format");
	effective_format = make_format_string_useful(fmt, parts.num);

	// "view"
#if GTK_CHECK_VERSION(4,0,0)

	// "model"
	store = g_list_store_new(G_TYPE_OBJECT);

	// selection model
	bool is_multi_sel = option_has(options, "multiline") or option_has(options, "selectmulti");
	if (is_multi_sel) {
		auto sel = gtk_multi_selection_new(G_LIST_MODEL(store));
		selection_model = GTK_SELECTION_MODEL(sel);
	} else {
		auto sel = gtk_single_selection_new(G_LIST_MODEL(store));
		gtk_single_selection_set_autoselect(sel, false);
		gtk_single_selection_set_can_unselect(sel, true);
		selection_model = GTK_SELECTION_MODEL(sel);
	}

	GtkWidget *view;
	if (parts.num > 1) {
		is_column_view = true;
		view = gtk_column_view_new(selection_model);

		for (auto &p: parts) {
			auto factory = gtk_signal_list_item_factory_new();
			factories.add(factory);
			g_signal_connect(factory, "setup", G_CALLBACK(on_gtk_list_item_setup), this);
			g_signal_connect(factory, "bind", G_CALLBACK(on_gtk_list_item_bind), this);

			auto col = gtk_column_view_column_new(p.c_str(), factory);
			columns.add(col);
			gtk_column_view_append_column(GTK_COLUMN_VIEW(view), GTK_COLUMN_VIEW_COLUMN(col));
		}
		if (columns.num >= 2 and effective_format.back() == 'B')
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
	auto types = CreateTypeList(effective_format);
	auto store = gtk_list_store_newv(types.num, &types[0]);

	auto view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

	g_signal_connect(G_OBJECT(view), "row-activated", G_CALLBACK(&on_gtk3_treeview_row_activated), this);

	// capture right click
	g_signal_connect(G_OBJECT(view), "button-press-event", G_CALLBACK(&on_gtk_list_right_button), this);
	gtk_widget_add_events(view, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(&on_gtk_list_select), this);


	// drag'n'drop reordering
	g_signal_connect(G_OBJECT(store), "row-inserted", G_CALLBACK(&on_gtk_list_row_inserted), this);
	g_signal_connect(G_OBJECT(store), "row-deleted", G_CALLBACK(&on_gtk_list_row_deleted), this);
#endif




	allow_change_messages = true;
	focusable = true;
	row_target = -1;

	// frame
	frame = sw;
	if (panel->border_width > 0 and !option_has(options, "nobar")) {
		frame = gtk_frame_new(nullptr);
#if GTK_CHECK_VERSION(4,0,0)
		gtk_frame_set_child(GTK_FRAME(frame), sw);
#else
		gtk_container_add(GTK_CONTAINER(frame), sw);
#endif
	}



#if GTK_CHECK_VERSION(4,0,0)
	// overlay to show drag'n'drop state
	auto ol = gtk_overlay_new();
	overlay_drawing_area = gtk_drawing_area_new();
	gtk_widget_set_halign(overlay_drawing_area, GTK_ALIGN_FILL);
	gtk_widget_set_valign(overlay_drawing_area, GTK_ALIGN_FILL);
	gtk_widget_set_can_target(overlay_drawing_area, false);
	gtk_overlay_set_child(GTK_OVERLAY(ol), view);
	gtk_overlay_add_overlay(GTK_OVERLAY(ol), overlay_drawing_area);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), ol);//view);

	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(overlay_drawing_area), on_gtk_list_overlay_draw, this, nullptr);
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
	set_options(get_option_from_title(title));
}

string ControlListView::get_string() {
	return "";
}

void ControlListView::__set_string(const string &str) {
	__add_string(str);
}

#if !GTK_CHECK_VERSION(4,0,0)
void set_list_cell(GtkListStore *store, GtkTreeIter &iter, int column, const string &str) {
	GType type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(store), column);
	if (type == G_TYPE_STRING) {
		gtk_list_store_set(store, &iter, column, sys_str(str), -1);
	} else if (type == G_TYPE_BOOLEAN) {
		gtk_list_store_set(store, &iter, column, (str == "1") or (str == "true"), -1);
	} else if (type == GDK_TYPE_PIXBUF) {
		GdkPixbuf *p = (GdkPixbuf*)get_gtk_image_paintable(str);
		if (p)
			gtk_list_store_set(store, &iter, column, p, -1);
		else if (str != "")
			msg_error("no image: " + str);
	}
}
#endif

void ControlListView::__add_string(const string& str) {
	allow_change_messages = false;
	auto parts = split_title(str);
#if GTK_CHECK_VERSION(4,0,0)
	//auto ob = gtk_string_object_new(str.c_str());
	//g_list_store_append(G_LIST_STORE(store), ob);
	auto row = g_list_store_new(G_TYPE_OBJECT);
	for (auto &p: parts) {
		auto ob = gtk_string_object_new(p.c_str());
		g_list_store_append(G_LIST_STORE(row), ob);
	}
	g_list_store_append(G_LIST_STORE(store), row);
#else
	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_list_store_append(store, &iter);
	for (int j=0; j<parts.num; j++)
		set_list_cell(store, iter, j, parts[j]);
#endif
	allow_change_messages = true;
}

void ControlListView::__set_int(int i) {
#if GTK_CHECK_VERSION(4,0,0)
	if (i >= 0)
		gtk_selection_model_select_item(selection_model, i, true);
	else
		gtk_selection_model_unselect_all(selection_model);
#else
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
#endif
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
#if GTK_CHECK_VERSION(4,0,0)
	auto parts = split_title(str);
	for (auto&& [i,p]: enumerate(parts))
		__update_cell(row, i, p, true);
#else
	auto parts = split_title(str);
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		for (int j=0; j<parts.num; j++)
			set_list_cell(store, iter, j, parts[j]);
#endif
}

void ControlListView::__remove_string(int row) {
	if (row < 0)
		return;
#if GTK_CHECK_VERSION(4,0,0)
	g_list_store_remove(G_LIST_STORE(store), row);
#else
	allow_change_messages = false;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		gtk_list_store_remove(store, &iter);
	allow_change_messages = true;
#endif
}

string ControlListView::get_cell(int row, int column) {
	if (row < 0)
		return "";
#if GTK_CHECK_VERSION(4,0,0)
	auto store_row = g_list_model_get_item(G_LIST_MODEL(store), row);
	if (!store_row)
		return "";
	auto obj = g_list_model_get_item(G_LIST_MODEL(store_row), column);
	if (!obj)
		return "";
	return gtk_string_object_get_string(GTK_STRING_OBJECT(obj));
	//return "";
#else
	GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;
	if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		return "";
	return tree_get_cell(store, iter, column);
#endif
}


#if GTK_CHECK_VERSION(4,0,0)
void ControlListView::__update_cell(int row, int column, const string& str, bool update_view) {
	auto store_row = g_list_model_get_item(G_LIST_MODEL(store), row);
	if (!store_row)
		return;
	/*g_list_store_remove(G_LIST_STORE(store_row), column);
	auto obj = gtk_string_object_new(str.c_str());
	g_list_store_insert(G_LIST_STORE(store_row), column, obj);
	//g_list_model_items_changed(G_LIST_MODEL(store), row, 1, 1);*/

	//auto it = g_list_model_get_object(G_LIST_MODEL(store_row), column);
	//g_object_set(it, "string", str.c_str(), nullptr);

	auto obj = gtk_string_object_new(str.c_str());
	g_list_store_splice(G_LIST_STORE(store_row), column, 1, (gpointer*)&obj, 1);

	if (update_view) {
		g_list_model_items_changed(G_LIST_MODEL(store), row, 1, 1);

		for (auto& i: _item_map_)
			if (i->row_in_model == row and i->column == column)
				gtk_list_view_update_item_view(this, column, i->widget, str.c_str());
	}

}
#endif

void ControlListView::__set_cell(int row, int column, const string& str) {
	dbo(format("SET CELL %d %d  %s", row, column, str));
	if (row < 0 or column < 0)
		return;
#if GTK_CHECK_VERSION(4,0,0)
	__update_cell(row, column, str, true);
#else
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, row))
		set_list_cell(store, iter, column, str);
#endif
}

Array<int> ControlListView::get_selection() {
	Array<int> selected;
#if GTK_CHECK_VERSION(4,0,0)
	auto bs = gtk_selection_model_get_selection(selection_model);
	for (guint64 i=0; i<gtk_bitset_get_size(bs); i++)
		selected.add(gtk_bitset_get_nth(bs, i));
#else
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	for (int i=0; ; i++) {
		GtkTreeIter iter;
		if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, i))
			return selected;
		if (gtk_tree_selection_iter_is_selected(sel, &iter))
			selected.add(i);
	}
#endif
	return selected;
}

void ControlListView::__set_selection(const Array<int> &selected) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_selection_model_unselect_all(selection_model);
	for (int i: selected)
		gtk_selection_model_select_item(selection_model, i, false);
#else
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_unselect_all(sel);
	for (int i: selected) {
		GtkTreeIter iter;
		if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, nullptr, i))
			continue;
		gtk_tree_selection_select_iter(sel, &iter);

	}
#endif
}

void ControlListView::__reset() {
	allow_change_messages = false;
#if GTK_CHECK_VERSION(4,0,0)
	g_list_store_remove_all(G_LIST_STORE(store));
	_item_map_.clear();
#else
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_list_store_clear(store);
#endif
	allow_change_messages = true;
}

void ControlListView::__set_option(const string &op, const string &value) {
#if GTK_CHECK_VERSION(4,0,0)
	if (op == "nobar") {
		if (is_column_view) {
			auto child = gtk_widget_get_first_child(widget);
			gtk_widget_set_visible(child, false);
		}
	} else if (op == "reorderable") {
		reorderable = true;

		GtkDropTarget *target = gtk_drop_target_new(G_TYPE_STRING, GDK_ACTION_COPY);
		g_signal_connect(target, "drop", G_CALLBACK(on_gtk_list_drop), this);
		g_signal_connect(target, "motion", G_CALLBACK(on_gtk_list_drop_motion), this);
		g_signal_connect(target, "leave", G_CALLBACK(on_gtk_list_drop_leave), this);
		gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(target));
	} else if (op == "singleclickactivate") {
		gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(widget), true);
	}
#else
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
		msg_error("ListView.singleclickactivate gtk3...");
	}
#endif
	if (op == "style") {
		add_css_class(value);
		//gtk_style_context_add_class(sc, "navigation-sidebar");
		//gtk_style_context_add_class(sc, "rich-list");
		//gtk_style_context_add_class(sc, "data-table");
	}
}

};

#endif
