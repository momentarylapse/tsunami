#include "hui.h"
#ifdef HUI_API_WIN

	#include <tchar.h>
	#include <commctrl.h>

/*
#include "../file/file.h"
#include <stdio.h>
#include <signal.h>
#ifdef HUI_API_WIN
	#include <shlobj.h>
	#include <winuser.h>
	#include <direct.h>
	#include <commctrl.h>
	#include <tchar.h>
	#pragma comment(lib,"winmm.lib")
	#pragma warning(disable : 4995)
#endif
#ifdef OS_LINUX
	#include <string.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/timeb.h>
	#include <time.h>
	#include <gdk/gdkx.h>
#endif*/

namespace hui
{

HuiMenu::HuiMenu()
{
	_HuiMakeUsable_();

	hMenu = CreateMenu();

}

HuiMenu::~HuiMenu()
{
}


// window coordinate system!
void HuiMenu::OpenPopup(HuiWindow *win, int x, int y)
{
#if 0
	tagPOINT pt;
	pt.x = pt.y = 0;
	ClientToScreen(win->hWnd, &pt);
	HMENU pm = CreateMenu();
	AppendMenu(pm, MF_STRING|MF_POPUP, (UINT)hMenu, _T(""));
	TrackPopupMenu(hMenu, 0, pt.x + x, pt.y + y, 0, win->hWnd, NULL);
	
	win->popup = this;
#endif
}


void HuiMenu::add(HuiControl *c)
{
}

// stupid function for HuiBui....
/*void HuiMenu::SetID(const string &id)
{
}*/

#if 0
void HuiMenu::AddItem(const string &name, const string & id)
{
#if 0
	HuiMenuItem i;
	AppendMenu(hMenu, MF_STRING, id, get_lang_sys(id, name, true));
	
	i.SubMenu = NULL;
	strcpy(i.Name, get_lang(id, name, false));
	i.ID = id;
	i.Enabled = true;
	i.IsSeparator = false;
	i.Checkable = false;
	i.Checked = false;
	Item.push_back(i);
#endif
}
#endif

int get_image_id(const string &image)
{
	if (image == "hui:open")	return STD_FILEOPEN;
	if (image == "hui:new")		return STD_FILENEW;
	if (image == "hui:save")	return STD_FILESAVE;

	if (image == "hui:copy")	return STD_COPY;
	if (image == "hui:paste")	return STD_PASTE;
	if (image == "hui:cut")		return STD_CUT;
	if (image == "hui:delete")	return STD_DELETE;
	if (image == "hui:find")	return STD_FIND;

	if (image == "hui:redo")	return STD_REDOW;
	if (image == "hui:undo")	return STD_UNDO;
	if (image == "hui:preferences")	return STD_PROPERTIES;

	if (image == "hui:help")	return STD_HELP;
	if (image == "hui:print")	return STD_PRINT;

	return STD_FILENEW;
}

#if 0
void HuiMenu::AddItemImage(const string &name, const string &image, const string &id)
{
#if 0
	HuiMenuItem i;
	AppendMenu(hMenu,MF_STRING,id,get_lang_sys(id,name,true));
	
	i.name = get_lang(id,name);
	i.id = id;
	item.add(i);
#endif
}

void HuiMenu::AddItemCheckable(const string &name, const string &id)
{
#if 0
	HuiMenuItem i;
	AppendMenu(hMenu,MF_STRING,id,get_lang_sys(id,name,true));
	
	i.name = get_lang(id,name);
	i.id = id;
	i.checkable = true;
	item.add(i);
#endif
}

void HuiMenu::AddSeparator()
{
#if 0
	HuiMenuItem i;
	AppendMenu(hMenu,MF_SEPARATOR,0,_T(""));
	
	i.is_separator = true;
	item.add(i);
#endif
}

void HuiMenu::AddSubMenu(const string &name, const string &id, HuiMenu *menu)
{
#if 0
	HuiMenuItem i;
	AppendMenu(hMenu,MF_STRING|MF_POPUP,(UINT)menu->hMenu,get_lang_sys(id,name));
		
	i.sub_menu = menu;
	i.name = get_lang(id,name);
	i.id = id;
	item.add(i);
#endif
}
#endif


/*void HuiMenu::CheckItem(const string &id, bool checked)
{
#if 0
	CheckMenuItem(hMenu,id,checked?MF_CHECKED:MF_UNCHECKED);
#endif
}

bool HuiMenu::IsItemChecked(const string &id)
{
	//CheckMenuItem(hMenu,id,checked?MF_CHECKED:MF_UNCHECKED);
	return false;
}

void HuiMenu::EnableItem(const string &id, bool enabled)
{
#if 0
	for (int i=0;i<Item.size();i++){
		if (Item[i].SubMenu)
			Item[i].SubMenu->EnableItem(id,enabled);
		if (Item[i].ID==id){
			Item[i].Enabled=enabled;
			// would be recursive by itself,....but who cares.
			if (enabled)
				EnableMenuItem(hMenu,id,MF_ENABLED);
			else{
				EnableMenuItem(hMenu,id,MF_DISABLED);
				EnableMenuItem(hMenu,id,MF_GRAYED);
			}
		}
	}
#endif
}

void HuiMenu::SetText(const string &id, const string &text)
{
#if 0
	for (int i=0;i<Item.size();i++){
		if (Item[i].SubMenu)
			Item[i].SubMenu->SetText(id,text);
		if (Item[i].ID==id){
			strcpy(Item[i].Name,text);
			ModifyMenu(hMenu,i,MF_STRING | MF_BYPOSITION,id,sys_str(text));
		}
	}
#endif
}*/

};

#endif
