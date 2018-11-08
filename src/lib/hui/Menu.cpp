/*----------------------------------------------------------------------------*\
| Hui menu                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "hui.h"
#include "Controls/MenuItem.h"
#include "Controls/MenuItemSeparator.h"
#include "Controls/MenuItemSubmenu.h"
#include "Controls/MenuItemToggle.h"

namespace hui
{

void DBDEL(const string &type, const string &id, void *p);
void DBDEL_DONE();

void Menu::__init__()
{
	new(this) Menu;
}

void Menu::__delete__()
{
	this->Menu::~Menu();
}

void Menu::clear()
{
	DBDEL("menu", "", this);
	for (Control *c: items)
		delete(c);
	items.clear();
	DBDEL_DONE();
}

void Menu::add(const string &name, const string &id)
{
	_add(new MenuItem(name, id));
}

void Menu::add_with_image(const string &name, const string &image, const string &id)
{
	_add(new MenuItem(name, id));
	items.back()->set_image(image);
}

void Menu::add_checkable(const string &name, const string &id)
{
	_add(new MenuItemToggle(name, id));
}

void Menu::add_separator()
{
	_add(new MenuItemSeparator());
}

void Menu::add_sub_menu(const string &name, const string &id, Menu *menu)
{
	if (menu)
		_add(new MenuItemSubmenu(name, menu, id));
}

void try_add_accel(GtkWidget *item, const string &id, Panel *p);

void Menu::set_panel(Panel *_panel)
{
	panel = _panel;
	for (Control *c: items){
		c->panel = panel;
		if (panel)
			try_add_accel(c->widget, c->id, panel);
		MenuItemSubmenu *s = dynamic_cast<MenuItemSubmenu*>(c);
		if (s)
			s->sub_menu->set_panel(panel);
	}
}

// only allow menu callback, if we are in layer 0 (if we don't edit it ourself)
int allow_signal_level = 0;

// stupid function for HuiBui....
void Menu::set_id(const string &id)
{
}

Menu *Menu::get_sub_menu_by_id(const string &id)
{
	for (Control *c: items){
		MenuItemSubmenu *s = dynamic_cast<MenuItemSubmenu*>(c);
		if (s){
			if (s->id == id)
				return s->sub_menu;
			Menu *m = s->sub_menu->get_sub_menu_by_id(id);
			if (m)
				return m;
		}
	}
	return nullptr;
}


void Menu::__update_language()
{
#if 0
	foreach(MenuItem &it, items){
		if (it.sub_menu)
			it.sub_menu->updateLanguage();
		if ((it.id.num == 0) || (it.is_separator))
			continue;
		bool enabled = it.enabled;
		/*  TODO
		#ifdef HUI_API_WIN
			if (strlen(get_lang(it.ID, "", true)) > 0){
				strcpy(it.Name, HuiGetLanguage(it.ID));
				ModifyMenu(m->hMenu, i, MF_STRING | MF_BYPOSITION, it.ID, get_lang_sys(it.ID, "", true));
			}
		#endif
		*/
		string s = get_lang(it.id, "", true);
		if (s.num > 0)
			SetText(it.id, s);
		msg_todo("HuiUpdateMenuLanguage (GTK) (menu bar)");
			//gtk_menu_item_set_label(GTK_MENU_ITEM(it.g_item), get_lang_sys(it.ID, "", true));
		EnableItem(it.id, enabled);
	}
#endif
}

Array<Control*> Menu::get_all_controls()
{
	Array<Control*> list = items;
	for (Control *c: items){
		MenuItemSubmenu *s = dynamic_cast<MenuItemSubmenu*>(c);
		if (s)
			list.append(s->sub_menu->get_all_controls());
	}
	return list;
}


void Menu::enable(const string &id, bool enabled)
{
	for (Control *c: items){
		if (c->id == id)
			c->enable(enabled);
		if (c->type == MENU_ITEM_SUBMENU)
			dynamic_cast<MenuItemSubmenu*>(c)->sub_menu->enable(id, enabled);
	}
}

void Menu::check(const string& id, bool checked)
{
	apply_foreach(id, [checked](Control *c){ c->check(checked); });
}


void Menu::apply_foreach(const string &_id, std::function<void(Control*)> f)
{
	/*for (Control *c: items)
		c->apply_foreach(_id, f);*/

	// FIXME: menu items don't really know their children.... inconsistent...argh
	auto list = get_all_controls();
	for (auto *c: list)
		if (c->id == _id){
			f(c);
		}

}

}
;

