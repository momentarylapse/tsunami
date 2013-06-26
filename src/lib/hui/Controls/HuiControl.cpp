/*
 * HuiControl.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControl.h"
#include "../hui.h"

HuiControl::HuiControl(int _type, const string &_id)
{
	type = _type;
	id = _id;
	win = NULL;
	enabled = true;
#ifdef HUI_API_WIN
	hWND = NULL;
#endif
#ifdef HUI_API_GTK
	widget = NULL;
	frame = NULL;
#endif
}

HuiControl::~HuiControl()
{
	if (win){
		for (int i=0;i<win->control.num;i++)
			if (win->control[i] == this)
				win->control.erase(i);
	}
#ifdef HUI_API_GTK
	if (widget)
		gtk_widget_destroy(widget);
#endif
}

#ifdef HUI_API_GTK

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
	if (!win){
		msg_error("HuiControl.Notify without win: " + id);
		return;
	}
	msg_db_m("Control.Notify", 2);
	win->_SetCurID_(id);
	if (id.num > 0){
		HuiEvent e = HuiEvent(id, message);
		_HuiSendGlobalCommand_(&e);
		e.is_default = is_default;
		win->_SendEvent_(&e);
	}
}

