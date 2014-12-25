/*----------------------------------------------------------------------------*\
| Hui menu                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "hui.h"
#include "Controls/HuiMenuItem.h"
#include "Controls/HuiMenuItemToggle.h"
#include "Controls/HuiMenuItemSubmenu.h"
#include "Controls/HuiMenuItemSeparator.h"

void HuiMenu::__init__()
{
	new(this) HuiMenu;
}

void HuiMenu::__delete__()
{
	this->~HuiMenu();
}

void HuiMenu::clear()
{
	foreach(HuiControl *c, item)
		delete(c);
	item.clear();
}

void HuiMenu::addItem(const string &name, const string &id)
{
	add(new HuiMenuItem(name, id));
}

void HuiMenu::addItemImage(const string &name, const string &image, const string &id)
{
	add(new HuiMenuItem(name, id));
	item.back()->setImage(image);
}

void HuiMenu::addItemCheckable(const string &name, const string &id)
{
	add(new HuiMenuItemToggle(name, id));
}

void HuiMenu::addSeparator()
{
	add(new HuiMenuItemSeparator());
}

void HuiMenu::addSubMenu(const string &name, const string &id, HuiMenu *menu)
{
	if (menu)
		add(new HuiMenuItemSubmenu(name, menu, id));
}

void HuiMenu::set_panel(HuiPanel *_panel)
{
	panel = _panel;
	foreach(HuiControl *c, item){
		c->panel = panel;
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s)
			s->sub_menu->set_panel(panel);
	}
}

// only allow menu callback, if we are in layer 0 (if we don't edit it ourself)
int allow_signal_level = 0;

// stupid function for HuiBui....
void HuiMenu::setID(const string &id)
{
}

HuiMenu *HuiMenu::getSubMenuByID(const string &id)
{
	foreach(HuiControl *c, item){
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s){
			if (s->id == id)
				return s->sub_menu;
			HuiMenu *m = s->sub_menu->getSubMenuByID(id);
			if (m)
				return m;
		}
	}
	return NULL;
}


void HuiMenu::updateLanguage()
{
	msg_db_f("UpdateMenuLanguage", 1);
#if 0
	foreach(HuiMenuItem &it, item){
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

Array<HuiControl*> HuiMenu::get_all_controls()
{
	Array<HuiControl*> list = item;
	foreach(HuiControl *c, item){
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s)
			list.append(s->sub_menu->get_all_controls());
	}
	return list;
}


void HuiMenu::enable(const string &id, bool enabled)
{
	foreach(HuiControl *c, item){
		if (c->id == id)
			c->enable(enabled);
		if (c->type == HuiKindMenuItemSubmenu)
			dynamic_cast<HuiMenuItemSubmenu*>(c)->sub_menu->enable(id, enabled);
	}
}
