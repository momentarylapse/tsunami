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
	void _cdecl add(const string &title, const string &image, const string &id);
	void _cdecl add_checkable(const string &title, const string &image, const string &id);
	void _cdecl add_menu(const string &title, const string &image, Menu *menu, const string &id);
	void _cdecl add_menu_by_id(const string &title, const string &image, const string &menu_id, const string &id);
	void _cdecl add_separator();
	void _cdecl reset();
	void _cdecl set_by_id(const string &id);
	void _cdecl from_source(const string &source);
	void _cdecl from_resource(Resource *r);

	void _add(Control *c);

	void apply_foreach(const string &id, std::function<void(Control*)> f);
};

};

#endif /* HUITOOLBAR_H_ */
