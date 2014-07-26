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

void HuiControl::Enable(bool _enabled)
{
}

void HuiControl::Hide(bool hidden)
{
}

void HuiControl::SetTooltip(const string& str)
{
}

void HuiControl::Focus()
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

void HuiControl::Enable(bool _enabled)
{
    enabled = _enabled;
	gtk_widget_set_sensitive(widget, enabled);
}

void HuiControl::Hide(bool hidden)
{
	if (hidden)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);
}

void HuiControl::SetTooltip(const string& str)
{
	gtk_widget_set_tooltip_text(widget, sys_str(str));
}

void HuiControl::Focus()
{
	gtk_widget_grab_focus(widget);
}

bool HuiControl::HasFocus()
{
	return gtk_widget_has_focus(widget);
}

void HuiControl::SetOptions(const string &options)
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
			gtk_widget_set_margin_left(get_frame(), 20);
		else if (eq >= 0){
			string a0 = aa.head(eq);
			string a1 = aa.tail(aa.num-eq-1);
			if (a0 == "width")
				width = a1._int();
			else if (a0 == "height")
				height = a1._int();
			else if (a0 == "margin-left")
				gtk_widget_set_margin_left(get_frame(), a1._int());
			else if (a0 == "margin-right")
				gtk_widget_set_margin_right(get_frame(), a1._int());
			else if (a0 == "margin-top")
				gtk_widget_set_margin_top(get_frame(), a1._int());
			else if (a0 == "margin-bottom")
				gtk_widget_set_margin_bottom(get_frame(), a1._int());
			else
				__SetOption(a0, a1);
		}else
			__SetOption(aa, "");
	}
	if ((width >= 0) || (height >= 0))
		gtk_widget_set_size_request(get_frame(), width, height);
}

void HuiControl::GetSize(int &w, int &h)
{
	w = gdk_window_get_width(gtk_widget_get_window(widget));
	h = gdk_window_get_height(gtk_widget_get_window(widget));
}

#endif

bool HuiControl::IsEnabled()
{
	return enabled;
}

void HuiControl::Reset()
{
	allow_signal_level ++;
	__Reset();
	allow_signal_level --;
}

void HuiControl::SetString(const string& str)
{
	allow_signal_level ++;
	__SetString(str);
	allow_signal_level --;
}

void HuiControl::AddString(const string& str)
{
	allow_signal_level ++;
	__AddString(str);
	allow_signal_level --;
}

void HuiControl::SetInt(int i)
{
	allow_signal_level ++;
	__SetInt(i);
	allow_signal_level --;
}

void HuiControl::SetFloat(float f)
{
	allow_signal_level ++;
	__SetFloat(f);
	allow_signal_level --;
}

void HuiControl::SetColor(const color& c)
{
	allow_signal_level ++;
	__SetColor(c);
	allow_signal_level --;
}

void HuiControl::AddChildString(int parent_row, const string& str)
{
	allow_signal_level ++;
	__AddChildString(parent_row, str);
	allow_signal_level --;
}

void HuiControl::ChangeString(int row, const string& str)
{
	allow_signal_level ++;
	__ChangeString(row, str);
	allow_signal_level --;
}

void HuiControl::SetCell(int row, int column, const string& str)
{
	allow_signal_level ++;
	__SetCell(row, column, str);
	allow_signal_level --;
}

void HuiControl::SetMultiSelection(Array<int>& sel)
{
	allow_signal_level ++;
	__SetMultiSelection(sel);
	allow_signal_level --;
}

void HuiControl::Check(bool checked)
{
	allow_signal_level ++;
	__Check(checked);
	allow_signal_level --;
}

void HuiControl::Notify(const string &message, bool is_default)
{
	if (allow_signal_level > 0)
		return;
	if (!panel){
		msg_error("HuiControl.Notify without panel: " + id);
		return;
	}
	msg_db_m("Control.Notify", 2);
	panel->_SetCurID_(id);
	if (id.num == 0)
		return;
	notify_push(this);
	HuiEvent e = HuiEvent(id, message);
	_HuiSendGlobalCommand_(&e);
	e.is_default = is_default;
	panel->_SendEvent_(&e);

	if (notify_is_deleted(this)){
		notify_pop();
		return;
	}

	HuiWindow *win = panel->win;
	if (this == win->main_input_control){
		if (message == "hui:mouse-move")
			win->OnMouseMove();
		else if (message == "hui:mouse-wheel")
			win->OnMouseWheel();
		else if (message == "hui:mouse-enter")
			win->OnMouseEnter();
		else if (message == "hui:mouse-leave")
			win->OnMouseLeave();
		else if (message == "hui:left-button-down")
			win->OnLeftButtonDown();
		else if (message == "hui:left-button-up")
			win->OnLeftButtonUp();
		else if (message == "hui:middle-button-down")
			win->OnMiddleButtonDown();
		else if (message == "hui:middle-button-up")
			win->OnMiddleButtonUp();
		else if (message == "hui:right-button-down")
			win->OnRightButtonDown();
		else if (message == "hui:right-button-up")
			win->OnRightButtonUp();
		else if (message == "hui:key-down"){
			win->OnKeyDown();
			WinTrySendByKeyCode(win, HuiGetEvent()->key_code);
		}else if (message == "hui:key-up")
			win->OnKeyUp();
		else if (message == "hui:draw")
			win->OnDraw();
	}else if (type == HuiKindMultilineEdit){
		if (message == "hui:key-down"){
			if (((HuiControlMultilineEdit*)this)->handle_keys)
				WinTrySendByKeyCode(win, HuiGetEvent()->key_code);
		}
	}
	notify_pop();
}

