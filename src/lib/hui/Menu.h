/*----------------------------------------------------------------------------*\
| Hui menu                                                                     |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_MENU_EXISTS_
#define _HUI_MENU_EXISTS_

#include "common.h"

namespace hui
{

class Panel;
class Menu;
class Control;


class Menu : public VirtualBase
{
public:
	Menu();
	~Menu();
	void _cdecl __init__();
	void _cdecl __delete__();
	void _cdecl clear();
	void _cdecl open_popup(Panel *panel);
	void _cdecl add(const string &name, const string &id);
	void _cdecl add_with_image(const string &name, const string &image, const string &id);
	void _cdecl add_checkable(const string &name, const string &id);
	void _cdecl add_separator();
	void _cdecl add_sub_menu(const string &name, const string &id, Menu *menu);
	void _cdecl enable(const string &id, bool enabled);
	void _cdecl check(const string &id, bool checked);
	void _cdecl set_id(const string &id);
	Menu *get_sub_menu_by_id(const string &id);

	void _add(Control *c);
	Array<Control*> get_all_controls();

	void __update_language();
	void set_panel(Panel *panel);
	
#ifdef HUI_API_GTK
	void gtk_realize();
	void gtk_unrealize();
	GtkWidget* widget;
#endif

#ifdef HUI_API_WIN
	HMENU hMenu;
#endif
	Array<Control*> items;
	Panel *panel;

	void apply_foreach(const string &id, std::function<void(Control*)> f);
};

};

#endif
