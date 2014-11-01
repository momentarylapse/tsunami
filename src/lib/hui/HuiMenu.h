/*----------------------------------------------------------------------------*\
| Hui menu                                                                     |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_MENU_EXISTS_
#define _HUI_MENU_EXISTS_

#include "hui_common.h"


class HuiPanel;
class HuiMenu;
class HuiControl;


class HuiMenu
{
public:
	HuiMenu();
	~HuiMenu();
	void _cdecl __init__();
	void _cdecl __delete__();
	void _cdecl clear();
	void _cdecl openPopup(HuiPanel *panel, int x, int y);
	void _cdecl addItem(const string &name, const string &id);
	void _cdecl addItemImage(const string &name, const string &image, const string &id);
	void _cdecl addItemCheckable(const string &name, const string &id);
	void _cdecl addSeparator();
	void _cdecl addSubMenu(const string &name, const string &id, HuiMenu *menu);
	void _cdecl enable(const string &id, bool enabled);
	void _cdecl setID(const string &id);
	HuiMenu *getSubMenuByID(const string &id);

	void add(HuiControl *c);
	Array<HuiControl*> get_all_controls();

	void updateLanguage();
	void set_panel(HuiPanel *panel);
	
#ifdef HUI_API_GTK
	void gtk_realize();
	void gtk_unrealize();
	GtkWidget* widget;
#endif

#ifdef HUI_API_WIN
	HMENU hMenu;
#endif
	Array<HuiControl*> item;
	HuiPanel *panel;
};

#endif
