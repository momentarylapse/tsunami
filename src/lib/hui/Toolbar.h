/*
 * HuiToolbar.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUITOOLBAR_H_
#define HUITOOLBAR_H_

#include "hui.h"

namespace hui
{

class Window;

class Toolbar
{
public:
	Toolbar(Window *win, bool vertical = false);
	virtual ~Toolbar();
	Window *win;
	Array<Control*> item;
	string id;
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
	void _cdecl addItemMenu(const string &title, const string &image, Menu *menu, const string &id);
	void _cdecl addItemMenuByID(const string &title, const string &image, const string &menu_id, const string &id);
	void _cdecl addSeparator();
	void _cdecl reset();
	void _cdecl setByID(const string &id);
	void _cdecl fromSource(const string &source);
	void _cdecl fromResource(Resource *r);

	void add(Control *c);
};

};

#endif /* HUITOOLBAR_H_ */
