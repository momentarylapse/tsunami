/*
 * HuiControl.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "Control.h"
#include "../language.h"
#include "../Window.h"
#include "../Painter.h"
#include "../../os/msg.h"
#include "../../image/color.h"

#include <gtk/gtk.h>

namespace hui
{

// Ownership:
// * Controls can be owned by 2 owners:
//   * direct parent Control (or Panel if no parent)
//   * both, if Panel is embedded
// * auto-delete when no owners left
// * keeps a reference to gtk widget

// add_child()
// - connect gtk widgets
// - link into children (ownership)
// remove_child()
// - disconnect gtk widgets (not deleting, since we keep a reference)
// - un-own child


void DBDEL_START(const string &type, const string &id, void *p);
void DBDEL_X(const string &);
void DBDEL_DONE();

#if GTK_CHECK_VERSION(4,0,0)
GAction *panel_get_action(Panel *panel, const string &id);
#endif

void control_link(Control *parent, shared<Control> child) {
	DBDEL_X("link: " + parent->id + " << " + child->id);
	parent->children.add(child);
	child->parent = parent;
}

void control_unlink(Control *parent, Control *child) {
	if (child->parent != parent)
		msg_error("control_unlink(): child.parent != parent");

	DBDEL_X("unlink: " + parent->id + " << " + child->id);
	child->parent = nullptr;
	for (int i=0; i<parent->children.num; i++)
		if (parent->children[i] == child)
			parent->children.erase(i);
	// child might be deleted now!
	// if still alive -> owned by child->panel
}

Control::Control(int _type, const string &_id) {
	type = _type;
	id = _id;
	panel = nullptr;
	parent = nullptr;
	enabled = true;
	visible = true;
	focusable = false;
	main_window_control = false;
	basic_internal_key_handling = true;
	user_key_handling = false;
	allow_global_key_shortcuts = true;
#ifdef HUI_API_WIN
	hWnd = nullptr;
#endif
#ifdef HUI_API_GTK
	widget = nullptr;
	frame = nullptr;
#endif
	indent = -1;
	min_width = -1;
	min_height = -1;
}

void Control::take_gtk_ownership() {
	if (auto f = frame)
		g_object_ref(f);
	if (auto w = widget)
		g_object_ref(w);
}

void Control::disable_event_handlers_rec() {
	if (widget)
		g_signal_handlers_disconnect_by_data(widget, this);
	for (auto cc: weak(children))
		cc->disable_event_handlers_rec();
}

void unset_widgets_rec(Control *c) {
	for (auto *cc: weak(c->children))
		unset_widgets_rec(cc);
	c->widget = nullptr;
}


Control::~Control() {
	DBDEL_START(i2s(type), id, this);
	if (widget)
		g_signal_handlers_disconnect_by_data(widget, this);
	if (auto f = frame)
		g_object_unref(f);
	if (auto w = widget)
		g_object_unref(w);

	auto _children = weak(children);

	for (auto *c: _children)
		remove_child(c);
		// -> calls control_unlink(this, c);

	DBDEL_DONE();
}

#ifdef HUI_API_GTK


GtkWidget *Control::get_frame() {
	if (frame)
		return frame;
	return widget;
}

void Control::enable(bool _enabled) {
	enabled = _enabled;
	if (widget)
		gtk_widget_set_sensitive(widget, _enabled);
	//msg_write("Control.enable " + (panel?panel->id:"") + ":" + id + "   " + b2s(enabled));

#if GTK_CHECK_VERSION(4,0,0)
	if (auto a = panel_get_action(panel, id)) {
		//msg_write("Action.enable " + id);
		g_simple_action_set_enabled(G_SIMPLE_ACTION(a), _enabled);
	}
#endif
}

void Control::hide(bool hidden) {
	visible = !hidden;
#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_set_visible(widget, !hidden);
#else
	if (hidden)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);
#endif
}

void Control::set_tooltip(const string& str) {
	gtk_widget_set_tooltip_text(widget, sys_str(str));
}

void Control::focus() {
	gtk_widget_grab_focus(widget);
}

bool Control::has_focus() {
	return gtk_widget_has_focus(widget);
}

color Control::get_color() {
	return Black;
}


static Array<Panel::SizeGroup> global_size_groups;

Panel::SizeGroup *get_size_group(Panel *panel, const string &name, int mode) {
	auto *size_groups = &panel->size_groups;
	if (name.head(1) == "/")
		size_groups = &global_size_groups;
	for (auto &g: *size_groups)
		if (g.name == name)
			return &g;
	Panel::SizeGroup gg;
	gg.name = name;
	gg.mode = mode;
	auto mm = GTK_SIZE_GROUP_HORIZONTAL;
	if (mode == 2)
		mm = GTK_SIZE_GROUP_VERTICAL;
	if (mode == 3)
		mm = GTK_SIZE_GROUP_BOTH;
	gg.group = gtk_size_group_new(mm);
	size_groups->add(gg);
	return &size_groups->back();
}

/*void set_style_for_widget(GtkWidget *widget, const string &id, const string &_css) {
	string css = "#" + id.replace(":", "") + _css;
	//msg_write(css);

	auto *css_provider = gtk_css_provider_new();

#if GTK_CHECK_VERSION(4,0,0)
	gtk_css_provider_load_from_data(css_provider, (char*)css.data, css.num);
#else
	GError *error = nullptr;
	gtk_css_provider_load_from_data(css_provider, (char*)css.data, css.num, &error);
	if (error) {
		msg_error(string("css: ") + error->message + " (" + css + ")");
		return;
	}
#endif

	auto *context = gtk_widget_get_style_context(widget);
	gtk_style_context_add_provider(context,
			GTK_STYLE_PROVIDER(css_provider),
			GTK_STYLE_PROVIDER_PRIORITY_USER);
}*/

Array<std::pair<string, string>> parse_options(const string &options) {
	Array<std::pair<string, string>> r;

	auto a = options.explode(",");

	for (string &aa: a) {
		int eq = aa.find("=");
		string op, val;
		if (eq < 0) {
			op = aa.replace("-", "");
		} else {
			op = aa.head(eq).replace("-", "");
			val = aa.tail(aa.num - eq - 1);
		}
		r.add(std::make_pair(op, val));
	}
	return r;
}

bool val_is_positive(const string &val, bool def) {
	if (val == "no" or val == "false")
		return false;
	if (val == "yes" or val == "true")
		return true;
	return def;
}

void Control::set_options(const string &options) {
	allow_signal_level ++;

	gtk_widget_set_name(widget, id.replace(":", "").c_str());

	auto ops = parse_options(options);

	for (auto x: ops) {
		auto op = x.first;
		auto val = x.second;

		if (op == "expandx") {
			gtk_widget_set_hexpand(widget, val_is_positive(val, true));
		} else if (op == "noexpandx") {
			gtk_widget_set_hexpand(widget, false);
		} else if (op == "expandy") {
			gtk_widget_set_vexpand(widget, val_is_positive(val, true));
		} else if (op == "noexpandy") {
			gtk_widget_set_vexpand(widget, false);
		} else if (op == "indent") {
			indent = 25;
		#if GTK_CHECK_VERSION(3,12,0)
			gtk_widget_set_margin_start(get_frame(), indent);
		#else
			gtk_widget_set_margin_left(get_frame(), indent);
		#endif
		} else if (op == "noindent") {
			indent = 0;
		#if GTK_CHECK_VERSION(3,12,0)
			gtk_widget_set_margin_start(get_frame(), 0);
		#else
			gtk_widget_set_margin_left(get_frame(), 0);
		#endif
		} else if (op == "grabfocus") {
			focusable = val_is_positive(val, true);
#if GTK_CHECK_VERSION(4,0,0)
			gtk_widget_set_focusable(widget, focusable);
			if (focusable) {
				gtk_widget_set_focus_on_click(widget, true);
				// maybe the dialog should choose the focus...?
				/*run_later(.01f, [this] {
					msg_write("  GRAB FOCUS " + b2s(gtk_widget_grab_focus(widget)) + "   " + id  + "  " + p2s(this));
				})*/;
			}
#else
			gtk_widget_set_can_focus(widget, focusable);
#endif
		} else if (op == "ignorefocus") {
			focusable = false;
#if GTK_CHECK_VERSION(4,0,0)
			gtk_widget_set_focusable(widget, false);
#else
			gtk_widget_set_can_focus(widget, false);
#endif
		} else if (op == "mainwindowcontrol") {
			main_window_control = true;
			set_options("grabfocus");
		} else if (op == "big") {
			add_css_class("hui-big-font");
			__set_option(op, val);
		} else if (op == "huge") {
			add_css_class("hui-huge-font");
			__set_option(op, val);
		} else if (op == "small") {
			add_css_class("hui-small-font");
			__set_option(op, val);
		} else if (op == "disabled") {
			enable(!val_is_positive(val, true));
		} else if (op == "enabled") {
			enable(val_is_positive(val, true));
		} else if (op == "hidden") {
			hide(val_is_positive(val, true));
		} else if (op == "visible") {
			hide(!val_is_positive(val, true));
		} else if ((op == "width") or (op == "minwidth")) {
			min_width = val._int();
			gtk_widget_set_size_request(get_frame(), min_width, min_height);
		} else if ((op == "height") or (op == "minheight")) {
			min_height = val._int();
			gtk_widget_set_size_request(get_frame(), min_width, min_height);
		} else if ((op == "marginleft") or (op == "indent")) {
			indent = val._int();
			//printf("indent %d\n", indent);
		#if GTK_CHECK_VERSION(3,12,0)
			gtk_widget_set_margin_start(get_frame(), val._int());
		#else
			gtk_widget_set_margin_left(get_frame(), val._int());
		#endif
		} else if (op == "marginright") {
		#if GTK_CHECK_VERSION(3,12,0)
			gtk_widget_set_margin_end(get_frame(), val._int());
		#else
			gtk_widget_set_margin_right(get_frame(), val._int());
		#endif
		} else if (op == "margintop") {
			gtk_widget_set_margin_top(get_frame(), val._int());
		} else if (op == "marginbottom") {
			gtk_widget_set_margin_bottom(get_frame(), val._int());
		} else if (op == "padding") {
			if (val._int() < 5)
				add_css_class("hui-no-padding");
			else
				add_css_class("hui-more-padding");

			//set_style_for_widget(widget, id, format("{padding: %dpx;}", val._int()));
		} else if ((op == "hgroup") or (op == "vgroup")) {
			if (panel) {
				auto g = get_size_group(panel, val, (op == "vgroup") ? 2 : 1);
				gtk_size_group_add_widget(g->group, get_frame());
			}
		} else if (op == "class") {
			add_css_class(val);
		} else {
			__set_option(op, val);
		}
	}

	allow_signal_level --;
}

void Control::get_size(int &w, int &h) {
#if GTK_CHECK_VERSION(4,0,0)
	w = gtk_widget_get_width(widget);
	h = gtk_widget_get_height(widget);
#else
	w = gdk_window_get_width(gtk_widget_get_window(widget));
	h = gdk_window_get_height(gtk_widget_get_window(widget));
#endif
}

#endif

bool Control::is_enabled() {
	return enabled;
}

void Control::reset() {
	allow_signal_level ++;
	__reset();
	allow_signal_level --;
}

void Control::set_string(const string& str) {
	allow_signal_level ++;
	__set_string(str);
	allow_signal_level --;
}

void Control::add_string(const string& str) {
	allow_signal_level ++;
	__add_string(str);
	allow_signal_level --;
}

void Control::set_int(int i) {
	allow_signal_level ++;
	__set_int(i);
	allow_signal_level --;
}

void Control::set_float(float f) {
	allow_signal_level ++;
	__set_float(f);
	allow_signal_level --;
}

void Control::set_color(const color& c) {
	allow_signal_level ++;
	__set_color(c);
	allow_signal_level --;
}

void Control::add_child_string(int parent_row, const string& str) {
	allow_signal_level ++;
	__add_child_string(parent_row, str);
	allow_signal_level --;
}

void Control::change_string(int row, const string& str) {
	allow_signal_level ++;
	__change_string(row, str);
	allow_signal_level --;
}

void Control::remove_string(int row) {
	allow_signal_level ++;
	__remove_string(row);
	allow_signal_level --;
}

void Control::set_cell(int row, int column, const string& str) {
	allow_signal_level ++;
	__set_cell(row, column, str);
	allow_signal_level --;
}

void Control::set_selection(const Array<int>& sel) {
	allow_signal_level ++;
	__set_selection(sel);
	allow_signal_level --;
}

void Control::check(bool checked) {
	allow_signal_level ++;
	__check(checked);
	allow_signal_level --;
}

void Control::notify(const string &message, bool is_default) {
	if (allow_signal_level > 0)
		return;
	if (!panel) {
		msg_error("HuiControl.Notify without panel: " + id);
		return;
	}
	panel->_set_cur_id_(id);
	if (id.num == 0)
		return;

	// prevent deleting while handling events
	shared<Control> temp_owner = this;

	Event e = Event(id, message);
	//_SendGlobalCommand_(&e);
	e.is_default = is_default;
	panel->_send_event_(&e);

	if (!main_window_control)
		return;

	Window *win = panel->win;
	if (message == EventID::MOUSE_MOVE) {
		win->on_mouse_move(get_event()->m);
	} else if (message == EventID::MOUSE_WHEEL) {
		win->on_mouse_wheel(get_event()->scroll);
	} else if (message == EventID::MOUSE_ENTER) {
		win->on_mouse_enter(get_event()->m);
	} else if (message == EventID::MOUSE_LEAVE) {
		win->on_mouse_leave();
	} else if (message == EventID::LEFT_BUTTON_DOWN) {
		win->on_left_button_down(get_event()->m);
	} else if (message == EventID::LEFT_BUTTON_UP) {
		win->on_left_button_up(get_event()->m);
	} else if (message == EventID::MIDDLE_BUTTON_DOWN) {
		win->on_middle_button_down(get_event()->m);
	} else if (message == EventID::MIDDLE_BUTTON_UP) {
		win->on_middle_button_up(get_event()->m);
	} else if (message == EventID::RIGHT_BUTTON_DOWN) {
		win->on_right_button_down(get_event()->m);
	} else if (message == EventID::RIGHT_BUTTON_UP) {
		win->on_right_button_up(get_event()->m);
	} else if (message == EventID::KEY_DOWN) {
		win->on_key_down(get_event()->key_code);
	} else if (message == EventID::KEY_UP) {
		win->on_key_up(get_event()->key_code);
	} else if (message == EventID::DRAW) {
		Painter p(win, id);
		win->on_draw(&p);
	}
}


void Control::apply_foreach(const string &_id, std::function<void(Control*)> f) {
	if ((id == _id) or (_id == "*"))
		f(this);
	for (int i=0; i<children.num; i++) // f might delete a child...!
		if (children[i]->panel == panel)
			children[i]->apply_foreach(_id, f);
}

void Control::add_css_class(const string &_class) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_add_css_class(widget, _class.c_str());
#else
	auto sc = gtk_widget_get_style_context(widget);
	gtk_style_context_add_class(sc, _class.c_str());
#endif
}

void Control::remove_css_class(const string &_class) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_remove_css_class(widget, _class.c_str());
#else
	auto sc = gtk_widget_get_style_context(widget);
	gtk_style_context_remove_class(sc, _class.c_str());
#endif
}

};
