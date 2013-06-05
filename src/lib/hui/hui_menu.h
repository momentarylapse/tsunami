/*----------------------------------------------------------------------------*\
| Hui menu                                                                     |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_MENU_EXISTS_
#define _HUI_MENU_EXISTS_




class HuiWindow;
class HuiMenu;

class HuiMenuItem
{
	public:
	HuiMenu *sub_menu;
	string image;
	string id;
	string name;
	bool enabled, is_separator, checked, checkable;
#ifdef HUI_API_GTK
	GtkWidget* widget;
#endif
	HuiMenuItem()
	{
		image = "";
		sub_menu = NULL;
		name = "";
		id = "";
		enabled = true;
		is_separator = false;
		checkable = false;
		checked = false;
	}
};

class HuiMenu
{
public:
	HuiMenu();
	~HuiMenu();
	void _cdecl Clear();
	void _cdecl OpenPopup(HuiWindow *win, int x, int y);
	void _cdecl AddItem(const string &name, const string &id);
	void _cdecl AddItemImage(const string &name, const string &image, const string &id);
	void _cdecl AddItemCheckable(const string &name, const string &id);
	void _cdecl AddSeparator();
	void _cdecl AddSubMenu(const string &name, const string &id, HuiMenu *menu);
	void _cdecl CheckItem(const string &id, bool checked);
	bool _cdecl IsItemChecked(const string &id);
	void _cdecl EnableItem(const string &id, bool enabled);
	void _cdecl SetText(const string &id, const string &text);
	void _cdecl SetID(const string &id);
	HuiMenu *GetSubMenuByID(const string &id);

	void UpdateLanguage();
	
#ifdef HUI_API_GTK
	void gtk_realize();
	void gtk_unrealize();
	GtkWidget* g_menu;
#endif

#ifdef HUI_API_WIN
	HMENU hMenu;
#endif
	Array<HuiMenuItem> item;
};

HuiMenu *_cdecl HuiCreateMenu();

#endif
