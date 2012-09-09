#include "hui.h"
#include "hui_internal.h"

void CHuiWindow::ToolbarSetCurrent(int index)
{
#ifdef HUI_API_WIN
	index = HuiToolBarTop; // ... m(-_-)m
#endif
	cur_toolbar = &toolbar[index];
}

// just a helper function
void AddToolbarItem(HuiToolbar *tb, const string &id, int type, CHuiMenu *menu)
{
	HuiToolbarItem i;
	i.id = id;
	i.type = type;
	i.enabled = true;
	i.menu = menu;
	tb->item.add(i);
}

// add a menu to the toolbar by resource id
void CHuiWindow::ToolbarAddItemMenuByID(const string &title, const string &tool_tip, const string &image, const string &menu_id, const string &id)
{
	CHuiMenu *menu = HuiCreateResourceMenu(menu_id);
	ToolbarAddItemMenu(title, tool_tip, image, menu,id);
}

// create and apply a toolbar bar resource id
void CHuiWindow::ToolbarSetByID(const string &id)
{
	msg_db_r("ToolBarSetByID",1);
	msg_db_m(id.c_str(),1);
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error("ToolBarSetByID  :~~(");
		msg_db_l(1);
		return;
	}
	ToolbarReset();
	//ToolBarConfigure(res->b_param[0], res->b_param[1]);
	foreach(res->cmd, cmd){
		if (cmd->type == "Item")
			ToolbarAddItem(get_lang(cmd->id, "", false), get_lang(cmd->id, "", false), cmd->image, cmd->id);
		else if (cmd->type == "ItemCheckable")
			ToolbarAddItemCheckable(get_lang(cmd->id, "", false), get_lang(cmd->id, "", false), cmd->image, cmd->id);
		else if (cmd->type == "ItemSeparator")
			ToolbarAddSeparator();
		else if (cmd->type == "ItemPopup"){
			string title = get_lang(cmd->id, "", false);
			ToolbarAddItemMenuByID(title, title, cmd->image, cmd->s_param[0], cmd->id);
		}
	}
	EnableToolbar(true);
	msg_db_m(":)",1);
	msg_db_l(1);
	return;
}

//    for all
bool CHuiWindow::_ToolbarIsEnabled_(const string &id)
{
	for (int t=0;t<4;t++)
		foreach(toolbar[t].item, it)
			if (id == it->id)
				return it->enabled;
	return false;
}

