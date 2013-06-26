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
void HuiToolbar::AddItem(const string &title, const string &image, const string &id)
{
	add(new HuiToolItemButton(title, image, id));
}

// add a checkable button
void HuiToolbar::AddItemCheckable(const string &title, const string &image, const string &id)
{
	add(new HuiToolItemToggleButton(title, image, id));
}

void HuiToolbar::AddItemMenu(const string &title, const string &image, HuiMenu *menu, const string &id)
{
	if (!menu)
		return;
	add(new HuiToolItemMenuButton(title, menu, image, id));
}

void HuiToolbar::AddItemMenuByID(const string &title, const string &image, const string &menu_id, const string &id)
{
	HuiMenu *menu = HuiCreateResourceMenu(menu_id);
	AddItemMenu(title, image, menu, id);
}

void HuiToolbar::AddSeparator()
{
	add(new HuiToolItemSeparator());
}

// remove all items from the toolbar
void HuiToolbar::Reset()
{
	for (int i=0;i<item.num;i++)
		delete(item[i]);
	item.clear();
}

// create and apply a toolbar bar resource id
void HuiToolbar::SetByID(const string &id)
{
	msg_db_r("Toolbar.SetByID",1);
	msg_db_m(id.c_str(),1);
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error("Toolbar.SetByID  :~~(");
		msg_db_l(1);
		return;
	}
	Reset();
	//Configure(res->b_param[0], res->b_param[1]);
	foreach(HuiResourceCommand &cmd, res->cmd){
		if (cmd.type == "Item"){
			AddItem(get_lang(cmd.id, "", false), cmd.image, cmd.id);
			item.back()->SetTooltip(get_lang(cmd.id, "", false));
		}else if (cmd.type == "ItemCheckable"){
			AddItemCheckable(get_lang(cmd.id, "", false), cmd.image, cmd.id);
			item.back()->SetTooltip(get_lang(cmd.id, "", false));
		}else if (cmd.type == "ItemSeparator"){
			AddSeparator();
		}else if (cmd.type == "ItemPopup"){
			string title = get_lang(cmd.id, "", false);
			AddItemMenuByID(title, cmd.image, cmd.s_param[0], cmd.id);
			item.back()->SetTooltip(title);
		}
	}
	Enable(true);
	msg_db_m(":)",1);
	msg_db_l(1);
	return;
}



