/*
 * HuiToolbar.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiToolbar.h"
#include "Controls/HuiToolItemButton.h"
#include "Controls/HuiToolItemMenuButton.h"
#include "Controls/HuiToolItemToggleButton.h"
#include "Controls/HuiToolItemSeparator.h"


// add a default button
void HuiToolbar::addItem(const string &title, const string &image, const string &id)
{
	add(new HuiToolItemButton(title, image, id));
}

// add a checkable button
void HuiToolbar::addItemCheckable(const string &title, const string &image, const string &id)
{
	add(new HuiToolItemToggleButton(title, image, id));
}

void HuiToolbar::addItemMenu(const string &title, const string &image, HuiMenu *menu, const string &id)
{
	if (!menu)
		return;
	add(new HuiToolItemMenuButton(title, menu, image, id));
}

void HuiToolbar::addItemMenuByID(const string &title, const string &image, const string &menu_id, const string &id)
{
	HuiMenu *menu = HuiCreateResourceMenu(menu_id);
	addItemMenu(title, image, menu, id);
}

void HuiToolbar::addSeparator()
{
	add(new HuiToolItemSeparator());
}

// remove all items from the toolbar
void HuiToolbar::reset()
{
	for (int i=0;i<item.num;i++)
		delete(item[i]);
	item.clear();
}

// create and apply a toolbar bar resource id
void HuiToolbar::setByID(const string &id)
{
	msg_db_f("Toolbar.SetByID",1);
	msg_db_m(id.c_str(),1);
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error("Toolbar.SetByID  :~~(");
		return;
	}
	reset();
	//Configure(res->b_param[0], res->b_param[1]);
	foreach(HuiResource &cmd, res->children){
		if (cmd.type == "Item"){
			addItem(get_lang(cmd.id, "", false), cmd.image, cmd.id);
			item.back()->setTooltip(get_lang(cmd.id, "", false));
		}else if (cmd.type == "ItemCheckable"){
			addItemCheckable(get_lang(cmd.id, "", false), cmd.image, cmd.id);
			item.back()->setTooltip(get_lang(cmd.id, "", false));
		}else if (cmd.type == "ItemSeparator"){
			addSeparator();
		}else if (cmd.type == "ItemPopup"){
			string title = get_lang(cmd.id, "", false);
			addItemMenuByID(title, cmd.image, cmd.s_param[0], cmd.id);
			item.back()->setTooltip(title);
		}
	}
	enable(true);
	msg_db_m(":)",1);
}



