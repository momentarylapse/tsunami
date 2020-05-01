#include "Controls/ControlButton.h"
#include "Controls/ControlCheckBox.h"
#include "Controls/ControlColorButton.h"
#include "Controls/ControlComboBox.h"
#include "Controls/ControlDrawingArea.h"
#include "Controls/ControlEdit.h"
#include "Controls/ControlExpander.h"
#include "Controls/ControlGrid.h"
#include "Controls/ControlGroup.h"
#include "Controls/ControlLabel.h"
#include "Controls/ControlListView.h"
#include "Controls/ControlMultilineEdit.h"
#include "Controls/ControlMenuButton.h"
#include "Controls/ControlPaned.h"
#include "Controls/ControlProgressBar.h"
#include "Controls/ControlRadioButton.h"
#include "Controls/ControlRevealer.h"
#include "Controls/ControlScroller.h"
#include "Controls/ControlSeparator.h"
#include "Controls/ControlSlider.h"
#include "Controls/ControlSpinButton.h"
#include "Controls/ControlTabControl.h"
#include "Controls/ControlToggleButton.h"
#include "Controls/ControlTreeView.h"
#include "hui.h"
#include "internal.h"
#ifdef HUI_API_GTK

#ifndef OS_WINDOWS
#include <pango/pangocairo.h>
#endif

namespace hui
{

GtkTreeIter dummy_iter;

void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;


//----------------------------------------------------------------------------------
// creating control items


#if !GTK_CHECK_VERSION(2,22,0)
	void gtk_table_get_size(GtkTable *table, guint *rows, guint *columns)
	{
		g_object_get(G_OBJECT(table), "n-rows", rows, nullptr);
		g_object_get(G_OBJECT(table), "n-columns", columns, nullptr);
	}
#endif

#if !GTK_CHECK_VERSION(3,0,0)
	GtkWidget *gtk_scale_new_with_range(GtkOrientation orientation, double min, double max, double step)
	{
		if (orientation == GTK_ORIENTATION_VERTICAL)
			return gtk_vscale_new_with_range(min, max, step);
		else
			return gtk_hscale_new_with_range(min, max, step);
	}

	void gtk_combo_box_text_remove_all(GtkComboBoxText *c)
	{
		GtkTreeModel *m = gtk_combo_box_get_model(GTK_COMBO_BOX(c));
		int n = gtk_tree_model_iter_n_children(m, nullptr);
		for (int i=0;i<n;i++)
			gtk_combo_box_text_remove(c, 0);
	}

	void gtk_combo_box_text_append(GtkComboBoxText *c, const char *id, const char *text)
	{
		gtk_combo_box_text_append_text(c, text);
	}

	GtkWidget *gtk_box_new(GtkOrientation orientation, int spacing)
	{
		if (orientation == GTK_ORIENTATION_VERTICAL)
			return gtk_vbox_new(false, spacing);
		else
			return gtk_hbox_new(false, spacing);
	}
#endif

void Panel::_insert_control_(Control *c, int x, int y) {
	GtkWidget *frame = c->get_frame();
	c->panel = this;
	if (cur_control) {
		cur_control->add(c, x, y);
	}else{
		root_control = c;
		// directly into the window...
		//gtk_container_add(GTK_CONTAINER(plugable), frame);
		if (plugable) {
			// this = HuiWindow...
			gtk_box_pack_start(GTK_BOX(plugable), frame, true, true, 0);
			gtk_container_set_border_width(GTK_CONTAINER(plugable), border_width);
		}
	}
	if (frame != c->widget)
		gtk_widget_show(frame);
	gtk_widget_show(c->widget);
	c->enabled = true;
}

Control *Panel ::_get_control_(const string &id) {
	if ((id.num == 0) and (cur_id.num > 0))
		return _get_control_(cur_id);

	// search backwards -> multiple AddText()s with identical ids
	//   will always set their own text

	Control *r = nullptr;
	apply_foreach(id, [&](Control *c) { r = c; });
	if (r)
		return r;

	if (id.num != 0) {
		// ...test if exists in menu/toolbar before reporting an error!
		//msg_error("hui: unknown id: '" + id + "'");
	}
	return nullptr;
}

Control *Panel::_get_control_by_widget_(GtkWidget *widget) {
	Control *r = nullptr;
	apply_foreach("*", [&](Control *c) { if (c->widget == widget) r = c; });
	return r;
}

string Panel::_get_id_by_widget_(GtkWidget *widget) {
	string r = "";
	apply_foreach("*", [&](Control *c) { if (c->widget == widget) r = c->id; });
	return r;
}



void NotifyWindowByWidget(Panel *panel, GtkWidget *widget, const string &message = "", bool is_default = true) {
	if (allow_signal_level > 0)
		return;
	string id = panel->_get_id_by_widget_(widget);
	panel->_set_cur_id_(id);
	if (id.num > 0) {
		Event e = Event(id, message);
		e.is_default = is_default;
		panel->_send_event_(&e);
	}
}

void SetImageById(Panel *panel, const string &id) {
	if ((id == "ok") or (id == "cancel") or (id == "apply")) {
		panel->set_image(id, "hui:" + id);
	} else if (id != "") {
		for (auto &c: panel->event_listeners)
			if ((c.id == id) and (c.image != ""))
				panel->set_image(id, c.image);
	}
}



void Panel::add_button(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlButton(title, id, this), x, y);

	SetImageById(this, id);
}

void Panel::add_color_button(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlColorButton(title, id), x, y);
}

void Panel::add_def_button(const string &title, int x, int y, const string &id) {
	add_button(title, x, y, id);
	set_options(id, "default");
}




void Panel::add_check_box(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlCheckBox(title, id), x, y);
}

void Panel::add_label(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlLabel(title, id), x, y);
}



void Panel::add_edit(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlEdit(title, id), x, y);
}

void Panel::add_multiline_edit(const string &title, int x, int y, const string &id) {
	auto *m = new ControlMultilineEdit(title, id);
	_insert_control_(m, x, y);
	if (win)
		if ((!win->main_input_control) and m->handle_keys)
			win->main_input_control = m;
}

void Panel::add_spin_button(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlSpinButton(title, id), x, y);
}

void Panel::add_group(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlGroup(title, id), x, y);
}

void Panel::add_combo_box(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlComboBox(title, id), x, y);
}

void Panel::add_toggle_button(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlToggleButton(title, id), x, y);
}

void Panel::add_radio_button(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlRadioButton(title, id, this), x, y);
}

void Panel::add_tab_control(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlTabControl(title, id, this), x, y);
}

void Panel::set_target(const string &id) {
	cur_control = nullptr;
	if (id.num > 0)
		cur_control = _get_control_(id);
}

void Panel::add_list_view(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlListView(title, id, this), x, y);
}

void Panel::add_tree_view(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlTreeView(title, id, this), x, y);
}

void Panel::add_icon_view(const string &title, int x, int y, const string &id) {
	msg_todo("AddIconView: deprecated");
	/*
	GetPartStrings(id,title);

	GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	GtkWidget *view;

	GtkListStore *model;
	model = gtk_list_store_new (2, G_TYPE_STRING,GDK_TYPE_PIXBUF);
	view=gtk_icon_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(view),0);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(view),1);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(view),130);
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(view), GTK_SELECTION_SINGLE);
	// react on double click
	g_signal_connect(G_OBJECT(view), "item-activated", G_CALLBACK(&OnGtkIconListActivate), this);
	if (OptionString.find("nobar") >= 0)
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), false);

	GtkWidget *frame = gtk_frame_new(NULL);
	//gtk_widget_set_size_request(frame,width-4,height-4);
	//gtk_fixed_put(GTK_FIXED(cur_cnt),frame,x,y);
	//gtk_container_add(GTK_CONTAINER(frame),c.win);
	gtk_container_add(GTK_CONTAINER(frame),sw);
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);
	_InsertControl_(view, x, y, id, HuiKindIconView, frame);*/
}

void Panel::add_progress_bar(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlProgressBar(title, id), x, y);
}

void Panel::add_slider(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlSlider(title, id), x, y);
}

void Panel::add_image(const string &title, int x, int y, const string &id) {
	msg_todo("AddImage: deprecated");
	/*GetPartStrings(id, title);
	GtkWidget *im;
	if (PartString[0].num > 0) {
		if (PartString[0][0] == '/')
			im = gtk_image_new_from_file(sys_str_f(PartString[0]));
		else
			im = gtk_image_new_from_file(sys_str_f(HuiAppDirectoryStatic + PartString[0]));
	}else
		im = gtk_image_new();
	_InsertControl_(im, x, y, id, HuiKindImage);*/
}


void Panel::add_drawing_area(const string &title, int x, int y, const string &id) {
	auto *da = new ControlDrawingArea(title, id);
	_insert_control_(da, x, y);
	if (win and !win->main_input_control)
		win->main_input_control = da;
}


void Panel::add_grid(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlGrid(title, id, this), x, y);
}

void Panel::add_expander(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlExpander(title, id), x, y);
}

void Panel::add_paned(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlPaned(title, id), x, y);
}

void Panel::add_scroller(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlScroller(title, id), x, y);
}

void Panel::add_separator(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlSeparator(title, id), x, y);
}

void Panel::add_revealer(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlRevealer(title, id), x, y);
}

void Panel::add_menu_button(const string &title, int x, int y, const string &id) {
	_insert_control_(new ControlMenuButton(title, id), x, y);
}

void Panel::embed_dialog(const string &id, int x, int y) {
	border_width = 8;

	Resource *res = GetResource(id);
	if (!res)
		return;
	if (res->type != "Dialog")
		return;
	if (res->children.num == 0)
		return;
	Resource rr = res->children[0];
	rr.x = x;
	rr.y = y;

	string parent_id;
	if (cur_control)
		parent_id = cur_control->id;
	_add_control(id, rr, parent_id);
}

void hui_rm_event(Array<EventListener> &event, Control *c) {
	for (int i=0; i<event.num; i++)
		if (event[i].id == c->id) {
			//msg_write("erase event");
			event.erase(i);
			i --;
		}
	for (Control *cc : c->children)
		hui_rm_event(event, cc);
}

void Panel::remove_control(const string &id) {
	Control *c = _get_control_(id);
	if (c) {
		hui_rm_event(event_listeners, c);
		delete(c);
	}
}



//----------------------------------------------------------------------------------
// drawing

// can handle calls from non-main threads
void Panel::redraw(const string &_id) {
	ControlDrawingArea *c = dynamic_cast<ControlDrawingArea*>(_get_control_(_id));
	if (c)
		c->redraw();
}

void Panel::redraw_rect(const string &_id, const rect &r) {
	ControlDrawingArea *c = dynamic_cast<ControlDrawingArea*>(_get_control_(_id));
	if (c)
		c->redraw_partial(r);
}

}


#endif
