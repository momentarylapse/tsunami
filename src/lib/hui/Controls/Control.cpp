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

void WinTrySendByKeyCode(Window *win, int key_code);

// safety feature... in case we delete the control while it notifies us
struct _HuiNotifyStackElement
{
	Control *c;
	bool deleted;
};
static Array<_HuiNotifyStackElement> _notify_stack_;
inline void notify_push(Control *c)
{
	_HuiNotifyStackElement e;
	e.c = c;
	e.deleted = false;
	_notify_stack_.add(e);
}
inline void notify_pop()
{
	_notify_stack_.pop();
}
inline void notify_set_del(Control *c)
{
	for (_HuiNotifyStackElement &e : _notify_stack_)
		if (e.c == c)
			e.deleted = true;
}
inline bool notify_is_deleted(Control *c)
{
	for (_HuiNotifyStackElement &e : _notify_stack_)
		if (e.c == c)
			return e.deleted;
	return false;
}

Control::Control(int _type, const string &_id)
{
	type = _type;
	id = _id;
	panel = NULL;
	parent = NULL;
	enabled = true;
#ifdef HUI_API_WIN
	hWnd = NULL;
#endif
#ifdef HUI_API_GTK
	widget = NULL;
	frame = NULL;
#endif
	grab_focus = false;
	indent = -1;
}

Control::~Control()
{
	notify_set_del(this);
	if (parent){
		for (int i=0;i<parent->children.num;i++)
			if (parent->children[i] == this)
				parent->children.erase(i);
	}
	while (children.num > 0){
		Control *c = children.pop();
		delete(c);
	}
	if (panel){
		for (int i=0;i<panel->controls.num;i++)
			if (panel->controls[i] == this)
				panel->controls.erase(i);
	}
#ifdef HUI_API_GTK
	if (widget)
		gtk_widget_destroy(widget);
#endif
}

#ifdef HUI_API_WIN

void Control::enable(bool _enabled)
{
}

void Control::hide(bool hidden)
{
}

void Control::setTooltip(const string& str)
{
}

void Control::focus()
{
}

#endif

#ifdef HUI_API_GTK


GtkWidget *Control::get_frame()
{
	if (frame)
		return frame;
	return widget;
}

void Control::enable(bool _enabled)
{
    enabled = _enabled;
	gtk_widget_set_sensitive(widget, enabled);
}

void Control::hide(bool hidden)
{
	if (hidden)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);
}

void Control::setTooltip(const string& str)
{
	gtk_widget_set_tooltip_text(widget, sys_str(str));
}

void Control::focus()
{
	gtk_widget_grab_focus(widget);
}

bool Control::hasFocus()
{
	return gtk_widget_has_focus(widget);
}

void Control::setOptions(const string &options)
{
	allow_signal_level ++;
	Array<string> a = options.explode(",");
	int width = -1;
	int height = -1;
	for (string &aa : a){
		int eq = aa.find("=");
		string op;
		if (eq < 0)
			op = aa.replace("-", "");
		else
			op = aa.head(eq).replace("-", "");

		if (op == "expandx")
			gtk_widget_set_hexpand(widget, true);
		else if (op == "noexpandx")
			gtk_widget_set_hexpand(widget, false);
		else if (op == "expandy")
			gtk_widget_set_vexpand(widget, true);
		else if (op == "noexpandy")
			gtk_widget_set_vexpand(widget, false);
		else if (op == "indent"){
			indent = 25;
#if GTK_CHECK_VERSION(3,12,0)
			gtk_widget_set_margin_start(get_frame(), indent);
#else
			gtk_widget_set_margin_left(get_frame(), indent);
#endif
		}else if (op == "noindent"){
			indent = 0;
#if GTK_CHECK_VERSION(3,12,0)
			gtk_widget_set_margin_start(get_frame(), 0);
#else
			gtk_widget_set_margin_left(get_frame(), 0);
#endif
		}else if (op == "grabfocus"){
			grab_focus = true;
			gtk_widget_set_can_focus(widget, true);
			gtk_widget_grab_focus(widget);
		}else if (eq >= 0){
			string a1 = aa.tail(aa.num-eq-1);
			if (op == "width")
				width = a1._int();
			else if (op == "height")
				height = a1._int();
			else if ((op == "marginleft") or (op == "indent")){
				indent = a1._int();
				printf("indent %d\n", indent);
#if GTK_CHECK_VERSION(3,12,0)
				gtk_widget_set_margin_start(get_frame(), a1._int());
#else
				gtk_widget_set_margin_left(get_frame(), a1._int());
#endif
			}else if (op == "marginright"){
#if GTK_CHECK_VERSION(3,12,0)
				gtk_widget_set_margin_end(get_frame(), a1._int());
#else
				gtk_widget_set_margin_right(get_frame(), a1._int());
#endif
			}else if (op == "margintop"){
				gtk_widget_set_margin_top(get_frame(), a1._int());
			}else if (op == "marginbottom"){
				gtk_widget_set_margin_bottom(get_frame(), a1._int());
			}else{
				__setOption(op, a1);
			}
		}else
			__setOption(op, "");
	}
	if ((width >= 0) or (height >= 0))
		gtk_widget_set_size_request(get_frame(), width, height);
	allow_signal_level --;
}

void Control::getSize(int &w, int &h)
{
	w = gdk_window_get_width(gtk_widget_get_window(widget));
	h = gdk_window_get_height(gtk_widget_get_window(widget));
}

#endif

bool Control::isEnabled()
{
	return enabled;
}

void Control::reset()
{
	allow_signal_level ++;
	__reset();
	allow_signal_level --;
}

void Control::setString(const string& str)
{
	allow_signal_level ++;
	__setString(str);
	allow_signal_level --;
}

void Control::addString(const string& str)
{
	allow_signal_level ++;
	__addString(str);
	allow_signal_level --;
}

void Control::setInt(int i)
{
	allow_signal_level ++;
	__setInt(i);
	allow_signal_level --;
}

void Control::setFloat(float f)
{
	allow_signal_level ++;
	__setFloat(f);
	allow_signal_level --;
}

void Control::setColor(const color& c)
{
	allow_signal_level ++;
	__setColor(c);
	allow_signal_level --;
}

void Control::addChildString(int parent_row, const string& str)
{
	allow_signal_level ++;
	__addChildString(parent_row, str);
	allow_signal_level --;
}

void Control::changeString(int row, const string& str)
{
	allow_signal_level ++;
	__changeString(row, str);
	allow_signal_level --;
}

void Control::removeString(int row)
{
	allow_signal_level ++;
	__removeString(row);
	allow_signal_level --;
}

void Control::setCell(int row, int column, const string& str)
{
	allow_signal_level ++;
	__setCell(row, column, str);
	allow_signal_level --;
}

void Control::setSelection(const Array<int>& sel)
{
	allow_signal_level ++;
	__setSelection(sel);
	allow_signal_level --;
}

void Control::check(bool checked)
{
	allow_signal_level ++;
	__check(checked);
	allow_signal_level --;
}

void Control::notify(const string &message, bool is_default)
{
	if (allow_signal_level > 0)
		return;
	if (!panel){
		msg_error("HuiControl.Notify without panel: " + id);
		return;
	}
	msg_db_m("Control.Notify", 2);
	panel->_set_cur_id_(id);
	if (id.num == 0)
		return;
	notify_push(this);
	Event e = Event(id, message);
	//_SendGlobalCommand_(&e);
	e.is_default = is_default;
	panel->_send_event_(&e);

	if (notify_is_deleted(this)){
		notify_pop();
		return;
	}

	Window *win = panel->win;
	if (this == win->main_input_control){
		if (message == "hui:mouse-move")
			win->onMouseMove();
		else if (message == "hui:mouse-wheel")
			win->onMouseWheel();
		else if (message == "hui:mouse-enter")
			win->onMouseEnter();
		else if (message == "hui:mouse-leave")
			win->onMouseLeave();
		else if (message == "hui:left-button-down")
			win->onLeftButtonDown();
		else if (message == "hui:left-button-up")
			win->onLeftButtonUp();
		else if (message == "hui:middle-button-down")
			win->onMiddleButtonDown();
		else if (message == "hui:middle-button-up")
			win->onMiddleButtonUp();
		else if (message == "hui:right-button-down")
			win->onRightButtonDown();
		else if (message == "hui:right-button-up")
			win->onRightButtonUp();
		else if (message == "hui:key-down"){
			win->onKeyDown();
			WinTrySendByKeyCode(win, GetEvent()->key_code);
		}else if (message == "hui:key-up")
			win->onKeyUp();
		else if (message == "hui:draw"){
			Painter p(win, id);
			win->onDraw(&p);
		}
	}else if (type == CONTROL_MULTILINEEDIT){
		if (message == "hui:key-down"){
			if (reinterpret_cast<ControlMultilineEdit*>(this)->handle_keys)
				WinTrySendByKeyCode(win, GetEvent()->key_code);
		}
	}
	notify_pop();
}

};
