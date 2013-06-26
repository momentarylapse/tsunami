/*
 * HuiToolbar.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUITOOLBAR_H_
#define HUITOOLBAR_H_

#include "hui.h"

class HuiWindow;

class HuiToolbar
{
public:
	HuiToolbar(HuiWindow *win, bool vertical = false);
	virtual ~HuiToolbar();
	HuiWindow *win;
	Array<HuiControl*> item;
	bool enabled;
	bool text_enabled;
	bool large_icons;
#ifdef HUI_API_WIN
	HWND hWnd;
#endif
#ifdef HUI_API_GTK
	GtkWidget *widget;
#endif

	void _cdecl Enable(bool enabled);
	void _cdecl Configure(bool text_enabled, bool large_icons);
	void _cdecl AddItem(const string &title, const string &image, const string &id);
	void _cdecl AddItemCheckable(const string &title, const string &image, const string &id);
	void _cdecl AddItemMenu(const string &title, const string &image, HuiMenu *menu, const string &id);
	void _cdecl AddItemMenuByID(const string &title, const string &image, const string &menu_id, const string &id);
	void _cdecl AddSeparator();
	void _cdecl Reset();
	void _cdecl SetByID(const string &id);

	void add(HuiControl *c);
};


#endif /* HUITOOLBAR_H_ */
