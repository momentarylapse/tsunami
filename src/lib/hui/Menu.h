/*----------------------------------------------------------------------------*\
| Hui menu                                                                     |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_MENU_EXISTS_
#define _HUI_MENU_EXISTS_

//#include "common.h"
#include "../base/pointer.h"
#include "Callback.h"
//#include <gtk/gtk.h>

typedef struct _GMenu GMenu;
typedef struct _GtkWidget GtkWidget;

namespace hui
{

class Panel;
class Menu;
class Control;


class Menu : public VirtualBase {
public:
	explicit Menu(Panel *p);
	~Menu() override;
	void _cdecl __init__(Panel *p);
	void _cdecl __delete__() override;
	void _cdecl clear();
	void _cdecl open_popup(Panel *panel);
	void _cdecl add(const string &name, const string &id);
	void _cdecl add_with_image(const string &name, const string &image, const string &id);
	void _cdecl add_checkable(const string &name, const string &id);
	void _cdecl add_separator();
	void _cdecl add_sub_menu(const string &name, const string &id, xfer<Menu> menu);
	void _cdecl enable(const string &id, bool enabled);
	void _cdecl check(const string &id, bool checked);
	void _cdecl set_id(const string &id);
	Menu *get_sub_menu_by_id(const string &id);

	void _add(shared<Control> c);
	Array<Control*> get_all_controls();

	void __update_language();
	void set_panel(Panel *panel);

	void gtk_realize();
	void gtk_unrealize();
//#if GTK_CHECK_VERSION(4,0,0)
	GMenu *gmenu;
//#else
	GtkWidget* widget;
//#endif

	shared_array<Control> items;
	Panel *panel;

	void apply_foreach(const string &id, std::function<void(Control*)> f);
};

};

#endif
