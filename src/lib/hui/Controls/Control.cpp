/*
 * HuiControl.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "Control.h"

#include "../hui.h"
#include "ControlMultilineEdit.h"

namespace hui
{


void DBDEL(const string &type, const string &id, void *p);
void DBDEL_DONE();

void WinTrySendByKeyCode(Window *win, int key_code);

// safety feature... in case we delete the control while it notifies us
struct _HuiNotifyStackElement {
	Control *c;
	bool deleted;
};

static Array<_HuiNotifyStackElement> _notify_stack_;

inline void notify_push(Control *c) {
	_HuiNotifyStackElement e;
	e.c = c;
	e.deleted = false;
	_notify_stack_.add(e);
}

inline void notify_pop() {
	_notify_stack_.pop();
}
inline void notify_set_del(Control *c) {
	for (_HuiNotifyStackElement &e : _notify_stack_)
		if (e.c == c)
			e.deleted = true;
}
inline bool notify_is_deleted(Control *c) {
	for (_HuiNotifyStackElement &e : _notify_stack_)
		if (e.c == c)
			return e.deleted;
	return false;
}

Control::Control(int _type, const string &_id) {
	type = _type;
	id = _id;
	panel = nullptr;
	parent = nullptr;
	enabled = true;
#ifdef HUI_API_WIN
	hWnd = nullptr;
#endif
#ifdef HUI_API_GTK
	widget = nullptr;
	frame = nullptr;
#endif
	grab_focus = false;
	indent = -1;
	min_width = -1;
	min_height = -1;
}

void unset_widgets_rec(Control *c) {
	for (auto *cc: c->children)
		unset_widgets_rec(cc);
	c->widget = nullptr;
}

Control::~Control() {
	notify_set_del(this);
	DBDEL("control", id, this);

#ifdef HUI_API_GTK
	//if (widget)
	//	gtk_widget_destroy(widget);
	//unset_widgets_rec(this);
#endif

	if (parent) {
		for (int i=0;i<parent->children.num;i++)
			if (parent->children[i] == this)
				parent->children.erase(i);
	}
	while (children.num > 0) {
		Control *c = children.pop();
		delete(c);
	}


	//msg_write("widget: " + p2s(widget));
#ifdef HUI_API_GTK
	if (widget)
		gtk_widget_destroy(widget);
	widget = nullptr;
	//unset_widgets_rec(this);
#endif
	DBDEL_DONE();
}

#ifdef HUI_API_WIN

void Control::enable(bool _enabled) {
}

void Control::hide(bool hidden) {
}

void Control::setTooltip(const string& str) {
}

void Control::focus() {
}

#endif

#ifdef HUI_API_GTK


GtkWidget *Control::get_frame() {
	if (frame)
		return frame;
	return widget;
}

void Control::enable(bool _enabled) {
    enabled = _enabled;
	gtk_widget_set_sensitive(widget, enabled);
}

void Control::hide(bool hidden) {
	if (hidden)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);
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

Panel::SizeGroup *get_size_group(Panel *panel, const string &name, int mode) {
	for (auto &g: panel->size_groups)
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
	panel->size_groups.add(gg);
	return &panel->size_groups.back();
}

void set_style_for_widget(GtkWidget *widget, const string &id, const string &_css) {
	string css = "#" + id.replace(":", "") + _css;
	//msg_write(css);
	GError *error = nullptr;

	auto *css_provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(css_provider, (char*)css.data, css.num, &error);
	if (error) {
		msg_error(string("css: ") + error->message + " (" + css + ")");
		return;
	}


	auto *context = gtk_widget_get_style_context(widget);
	gtk_style_context_add_provider(context,
					                                GTK_STYLE_PROVIDER(css_provider),
													GTK_STYLE_PROVIDER_PRIORITY_USER);
	/*gtk_style_context_add_provider_for_screen
	                               (gdk_screen_get_default(),
	                                GTK_STYLE_PROVIDER(css_provider),
									GTK_STYLE_PROVIDER_PRIORITY_USER);*/
}

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
			grab_focus = val_is_positive(val, true);
			gtk_widget_set_can_focus(widget, grab_focus);
			if (grab_focus)
				gtk_widget_grab_focus(widget);
		} else if (op == "ignorefocus") {
			grab_focus = false;
			gtk_widget_set_can_focus(widget, false);
		} else if (op == "big") {
			set_style_for_widget(widget, id, "{font-size: 150%}");
		} else if (op == "huge") {
			set_style_for_widget(widget, id, "{font-size: 180%}");
		} else if (op == "small") {
			set_style_for_widget(widget, id, "{font-size: 75%}");
		} else if (op == "disabled") {
			enable(false);
		} else if (op == "enabled") {
			enable(true);
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
			set_style_for_widget(widget, id, format("{padding: %dpx}", val._int()));
		} else if ((op == "hgroup") or (op == "vgroup")) {
			if (panel) {
				auto g = get_size_group(panel, val, (op == "vgroup") ? 2 : 1);
				gtk_size_group_add_widget(g->group, get_frame());
			}
		} else {
			__set_option(op, val);
		}
	}

	allow_signal_level --;
}

void Control::get_size(int &w, int &h) {
	w = gdk_window_get_width(gtk_widget_get_window(widget));
	h = gdk_window_get_height(gtk_widget_get_window(widget));
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
	notify_push(this);
	Event e = Event(id, message);
	//_SendGlobalCommand_(&e);
	e.is_default = is_default;
	panel->_send_event_(&e);

	if (notify_is_deleted(this)) {
		notify_pop();
		return;
	}

	Window *win = panel->win;
	if (this == win->main_input_control) {
		if (message == "hui:mouse-move")
			win->on_mouse_move();
		else if (message == "hui:mouse-wheel")
			win->on_mouse_wheel();
		else if (message == "hui:mouse-enter")
			win->on_mouse_enter();
		else if (message == "hui:mouse-leave")
			win->on_mouse_leave();
		else if (message == "hui:left-button-down")
			win->on_left_button_down();
		else if (message == "hui:left-button-up")
			win->on_left_button_up();
		else if (message == "hui:middle-button-down")
			win->on_middle_button_down();
		else if (message == "hui:middle-button-up")
			win->on_middle_button_up();
		else if (message == "hui:right-button-down")
			win->on_right_button_down();
		else if (message == "hui:right-button-up")
			win->on_right_button_up();
		else if (message == "hui:key-down") {
			win->on_key_down();
			WinTrySendByKeyCode(win, GetEvent()->key_code);
		} else if (message == "hui:key-up")
			win->on_key_up();
		else if (message == "hui:draw") {
			Painter p(win, id);
			win->on_draw(&p);
		}
	} else if (type == CONTROL_MULTILINEEDIT) {
		if (message == "hui:key-down") {
			if (reinterpret_cast<ControlMultilineEdit*>(this)->handle_keys)
				WinTrySendByKeyCode(win, GetEvent()->key_code);
		}
	}
	notify_pop();
}


void Control::apply_foreach(const string &_id, std::function<void(Control*)> f) {
	if ((id == _id) or (_id == "*"))
		f(this);
	for (Control *c: children)
		if (c->panel == panel)
			c->apply_foreach(_id, f);
}

};
