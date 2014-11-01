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

	void _cdecl enable(bool enabled);
	void _cdecl configure(bool text_enabled, bool large_icons);
	void _cdecl addItem(const string &title, const string &image, const string &id);
	void _cdecl addItemCheckable(const string &title, const string &image, const string &id);
	void _cdecl addItemMenu(const string &title, const string &image, HuiMenu *menu, const string &id);
	void _cdecl addItemMenuByID(const string &title, const string &image, const string &menu_id, const string &id);
	void _cdecl addSeparator();
	void _cdecl reset();
	void _cdecl setByID(const string &id);

	void add(HuiControl *c);
};


#endif /* HUITOOLBAR_H_ */
