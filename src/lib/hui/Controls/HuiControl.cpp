/*
 * HuiControl.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControl.h"
#include "../hui.h"
#include "HuiControlMultilineEdit.h"

void WinTrySendByKeyCode(HuiWindow *win, int key_code);

// safety feature... in case we delete the control while it notifies us
struct _HuiNotifyStackElement
{
	HuiControl *c;
	bool deleted;
};
static Array<_HuiNotifyStackElement> _notify_stack_;
inline void notify_push(HuiControl *c)
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
inline void notify_set_del(HuiControl *c)
{
	foreach(_HuiNotifyStackElement &e, _notify_stack_)
		if (e.c == c)
			e.deleted = true;
}
inline bool notify_is_deleted(HuiControl *c)
{
	foreach(_HuiNotifyStackElement &e, _notify_stack_)
		if (e.c == c)
			return e.deleted;
	return false;
}

HuiControl::HuiControl(int _type, const string &_id)
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
}

HuiControl::~HuiControl()
{
	notify_set_del(this);
	if (parent){
		for (int i=0;i<parent->children.num;i++)
			if (parent->children[i] == this)
				parent->children.erase(i);
	}
	while (children.num > 0){
		HuiControl *c = children.pop();
		delete(c);
	}
	if (panel){
		for (int i=0;i<panel->control.num;i++)
			if (panel->control[i] == this)
				panel->control.erase(i);
	}
#ifdef HUI_API_GTK
	if (widget)
		gtk_widget_destroy(widget);
#endif
}

#ifdef HUI_API_WIN

void HuiControl::enable(bool _enabled)
{
}

void HuiControl::hide(bool hidden)
{
}

void HuiControl::setTooltip(const string& str)
{
}

void HuiControl::focus()
{
}

#endif

#ifdef HUI_API_GTK


GtkWidget *HuiControl::get_frame()
{
	if (frame)
		return frame;
	return widget;
}

void HuiControl::enable(bool _enabled)
{
    enabled = _enabled;
	gtk_widget_set_sensitive(widget, enabled);
}

void HuiControl::hide(bool hidden)
{
	if (hidden)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);
}

void HuiControl::setTooltip(const string& str)
{
	gtk_widget_set_tooltip_text(widget, sys_str(str));
}

void HuiControl::focus()
{
	gtk_widget_grab_focus(widget);
}

bool HuiControl::hasFocus()
{
	return gtk_widget_has_focus(widget);
}

void HuiControl::setOptions(const string &options)
{
	Array<string> a = options.explode(",");
	int width = -1;
	int height = -1;
	foreach(string &aa, a){
		int eq = aa.find("=");
		if (aa == "expandx")
			gtk_widget_set_hexpand(widget, true);
		else if (aa == "noexpandx")
			gtk_widget_set_hexpand(widget, false);
		else if (aa == "expandy")
			gtk_widget_set_vexpand(widget, true);
		else if (aa == "noexpandy")
			gtk_widget_set_vexpand(widget, false);
		else if (aa == "indent")
#if GTK_CHECK_VERSION(3,12,0)
			gtk_widget_set_margin_start(get_frame(), 20);
#else
			gtk_widget_set_margin_left(get_frame(), 20);
#endif
		else if (eq >= 0){
			string a0 = aa.head(eq);
			string a1 = aa.tail(aa.num-eq-1);
			if (a0 == "width")
				width = a1._int();
			else if (a0 == "height")
				height = a1._int();
			else if (a0 == "margin-left"){
#if GTK_CHECK_VERSION(3,12,0)
				gtk_widget_set_margin_start(get_frame(), 20);
#else
				gtk_widget_set_margin_left(get_frame(), a1._int());
#endif
			}else if (a0 == "margin-right"){
#if GTK_CHECK_VERSION(3,12,0)
				gtk_widget_set_margin_end(get_frame(), a1._int());
#else
				gtk_widget_set_margin_right(get_frame(), a1._int());
#endif
			}else if (a0 == "margin-top"){
				gtk_widget_set_margin_top(get_frame(), a1._int());
			}else if (a0 == "margin-bottom"){
				gtk_widget_set_margin_bottom(get_frame(), a1._int());
			}else{
				__setOption(a0, a1);
			}
		}else
			__setOption(aa, "");
	}
	if ((width >= 0) or (height >= 0))
		gtk_widget_set_size_request(get_frame(), width, height);
}

void HuiControl::getSize(int &w, int &h)
{
	w = gdk_window_get_width(gtk_widget_get_window(widget));
	h = gdk_window_get_height(gtk_widget_get_window(widget));
}

#endif

bool HuiControl::isEnabled()
{
	return enabled;
}

void HuiControl::reset()
{
	allow_signal_level ++;
	__reset();
	allow_signal_level --;
}

void HuiControl::setString(const string& str)
{
	allow_signal_level ++;
	__setString(str);
	allow_signal_level --;
}

void HuiControl::addString(const string& str)
{
	allow_signal_level ++;
	__addString(str);
	allow_signal_level --;
}

void HuiControl::setInt(int i)
{
	allow_signal_level ++;
	__setInt(i);
	allow_signal_level --;
}

void HuiControl::setFloat(float f)
{
	allow_signal_level ++;
	__setFloat(f);
	allow_signal_level --;
}

void HuiControl::setColor(const color& c)
{
	allow_signal_level ++;
	__setColor(c);
	allow_signal_level --;
}

void HuiControl::addChildString(int parent_row, const string& str)
{
	allow_signal_level ++;
	__addChildString(parent_row, str);
	allow_signal_level --;
}

void HuiControl::changeString(int row, const string& str)
{
	allow_signal_level ++;
	__changeString(row, str);
	allow_signal_level --;
}

void HuiControl::removeString(int row)
{
	allow_signal_level ++;
	__removeString(row);
	allow_signal_level --;
}

void HuiControl::setCell(int row, int column, const string& str)
{
	allow_signal_level ++;
	__setCell(row, column, str);
	allow_signal_level --;
}

void HuiControl::setSelection(Array<int>& sel)
{
	allow_signal_level ++;
	__setSelection(sel);
	allow_signal_level --;
}

void HuiControl::check(bool checked)
{
	allow_signal_level ++;
	__check(checked);
	allow_signal_level --;
}

void HuiControl::notify(const string &message, bool is_default)
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
	HuiEvent e = HuiEvent(id, message);
	_HuiSendGlobalCommand_(&e);
	e.is_default = is_default;
	panel->_send_event_(&e);

	if (notify_is_deleted(this)){
		notify_pop();
		return;
	}

	HuiWindow *win = panel->win;
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
			WinTrySendByKeyCode(win, HuiGetEvent()->key_code);
		}else if (message == "hui:key-up")
			win->onKeyUp();
		else if (message == "hui:draw")
			win->onDraw();
	}else if (type == HUI_KIND_MULTILINEEDIT){
		if (message == "hui:key-down"){
			if (((HuiControlMultilineEdit*)this)->handle_keys)
				WinTrySendByKeyCode(win, HuiGetEvent()->key_code);
		}
	}
	notify_pop();
}

