/*
 * HuiToolbar.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "Toolbar.h"

#include "Controls/ToolItemButton.h"
#include "Controls/ToolItemMenuButton.h"
#include "Controls/ToolItemSeparator.h"
#include "Controls/ToolItemToggleButton.h"

namespace hui
{

Menu *_create_res_menu_(const string &ns, Resource *res);


// add a default button
void Toolbar::addItem(const string &title, const string &image, const string &id)
{
	add(new ToolItemButton(title, image, id));
}

// add a checkable button
void Toolbar::addItemCheckable(const string &title, const string &image, const string &id)
{
	add(new ToolItemToggleButton(title, image, id));
}

void Toolbar::addItemMenu(const string &title, const string &image, Menu *menu, const string &id)
{
	if (!menu)
		return;
	add(new ToolItemMenuButton(title, menu, image, id));
	menu->set_panel(win);
}

void Toolbar::addItemMenuByID(const string &title, const string &image, const string &menu_id, const string &id)
{
	Menu *menu = CreateResourceMenu(menu_id);
	addItemMenu(title, image, menu, id);
}

void Toolbar::addSeparator()
{
	add(new ToolItemSeparator());
}

// remove all items from the toolbar
void Toolbar::reset()
{
	for (int i=0;i<item.num;i++)
		delete(item[i]);
	item.clear();
}

// create and apply a toolbar bar resource id
void Toolbar::setByID(const string &id)
{
	msg_db_m(id.c_str(),1);
	Resource *res = GetResource(id);
	if (!res){
		msg_error("Toolbar.SetByID  :~~(");
		return;
	}
	reset();
	//Configure(res->b_param[0], res->b_param[1]);
	for (Resource &cmd: res->children){
		if (cmd.type == "Item"){
			if (sa_contains(cmd.options, "checkable"))
				addItemCheckable(get_lang(id, cmd.id, "", false), cmd.image, cmd.id);
			else
				addItem(get_lang(id, cmd.id, "", false), cmd.image, cmd.id);
			item.back()->setTooltip(GetLanguageT(id, cmd.id));
		}else if (cmd.type == "ItemCheckable"){
			addItemCheckable(get_lang(id, cmd.id, "", false), cmd.image, cmd.id);
			item.back()->setTooltip(GetLanguageT(id, cmd.id));
		}else if ((cmd.type == "ItemSeparator") or (cmd.type == "Separator")){
			addSeparator();
		}else if (cmd.type == "ItemPopup"){
			string title = get_lang(id, cmd.id, "", false);
			bool ok = false;
			for (string &o: cmd.options)
				if (o.find("menu=") == 0){
					addItemMenuByID(title, cmd.image, o.substr(5, -1), cmd.id);
					item.back()->setTooltip(GetLanguageT(id, cmd.id));
					ok = true;
				}
			if ((!ok) and (cmd.children.num > 0)){
				addItemMenu(title, cmd.image, _create_res_menu_(id, &cmd), cmd.id);
				item.back()->setTooltip(GetLanguageT(id, cmd.id));
			}
		}
	}
	enable(true);
	msg_db_m(":)",1);
}

};

