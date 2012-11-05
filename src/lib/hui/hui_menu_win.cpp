#include "hui.h"
#ifdef HUI_API_WIN

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


CHuiMenu::CHuiMenu()
{
	msg_db_r("CHuiMenu()", 1);
	_HuiMakeUsable_();

	hMenu = CreateMenu();
	
	msg_db_l(1);
}

CHuiMenu::~CHuiMenu()
{
}


// window coordinate system!
void CHuiMenu::OpenPopup(CHuiWindow *win, int x, int y)
{
	msg_db_r("CHuiMenu::OpenPopup", 1);
	tagPOINT pt;
	pt.x = pt.y = 0;
	ClientToScreen(win->hWnd, &pt);
	HMENU pm = CreateMenu();
	AppendMenu(pm, MF_STRING|MF_POPUP, (UINT)hMenu, _T(""));
	TrackPopupMenu(hMenu, 0, pt.x + x, pt.y + y, 0, win->hWnd, NULL);
	
	win->Popup = this;
	msg_db_l(1);
}

// stupid function for HuiBui....
void CHuiMenu::SetID(int id)
{
}

void CHuiMenu::AddItem(const char *name, int id)
{
	sHuiMenuItem i;
	AppendMenu(hMenu, MF_STRING, id, get_lang_sys(id, name, true));
	
	i.SubMenu = NULL;
	strcpy(i.Name, get_lang(id, name, false));
	i.ID = id;
	i.Enabled = true;
	i.IsSeparator = false;
	i.Checkable = false;
	i.Checked = false;
	Item.push_back(i);
}

int get_image_id(int image)
{
	if (image==HuiImageOpen)	return STD_FILEOPEN;
	if (image==HuiImageNew)		return STD_FILENEW;
	if (image==HuiImageSave)	return STD_FILESAVE;

	if (image==HuiImageCopy)	return STD_COPY;
	if (image==HuiImagePaste)	return STD_PASTE;
	if (image==HuiImageCut)		return STD_CUT;
	if (image==HuiImageDelete)	return STD_DELETE;
	if (image==HuiImageFind)	return STD_FIND;

	if (image==HuiImageRedo)	return STD_REDOW;
	if (image==HuiImageUndo)	return STD_UNDO;
	if (image==HuiImagePreferences)	return STD_PROPERTIES;

	if (image==HuiImageHelp)	return STD_HELP;
	if (image==HuiImagePrint)	return STD_PRINT;

	return STD_FILENEW;
}

void CHuiMenu::AddItemImage(const char *name,int image,int id)
{
	sHuiMenuItem i;
	AppendMenu(hMenu,MF_STRING,id,get_lang_sys(id,name,true));
	
	i.SubMenu=NULL;
	strcpy(i.Name,get_lang(id,name,false));
	i.ID=id;
	i.Enabled=true;
	i.IsSeparator=false;
	i.Checkable=false;
	i.Checked=false;
	Item.push_back(i);
}

void CHuiMenu::AddItemCheckable(const char *name,int id)
{
	sHuiMenuItem i;
	AppendMenu(hMenu,MF_STRING,id,get_lang_sys(id,name,true));
	
	i.SubMenu=NULL;
	strcpy(i.Name,get_lang(id,name,false));
	i.ID=id;
	i.Enabled=true;
	i.IsSeparator=false;
	i.Checkable=true;
	i.Checked=false;
	Item.push_back(i);
}

void CHuiMenu::AddSeparator()
{
	sHuiMenuItem i;
	AppendMenu(hMenu,MF_SEPARATOR,0,_T(""));
	
	i.SubMenu=NULL;
	i.ID=-1;
	i.Enabled=true;
	i.IsSeparator=true;
	Item.push_back(i);
}

void CHuiMenu::AddSubMenu(const char *name,int id,CHuiMenu *menu)
{
	sHuiMenuItem i;
	AppendMenu(hMenu,MF_STRING|MF_POPUP,(UINT)menu->hMenu,get_lang_sys(id,name));
		
	i.SubMenu=menu;
	strcpy(i.Name,get_lang(id,name));
	i.ID=id;
	i.Enabled=true;
	i.IsSeparator=false;
	Item.push_back(i);
}

// only allow menu callback, if we are in layer 0 (if we don't edit it ourself)
int allow_signal_level=0;

void CHuiMenu::CheckItem(int id,bool checked)
{
	CheckMenuItem(hMenu,id,checked?MF_CHECKED:MF_UNCHECKED);
}

bool CHuiMenu::IsItemChecked(int id)
{
	//CheckMenuItem(hMenu,id,checked?MF_CHECKED:MF_UNCHECKED);
	return false;
}

void CHuiMenu::EnableItem(int id,bool enabled)
{
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
}

void CHuiMenu::SetText(int id,const char *text)
{
	for (int i=0;i<Item.size();i++){
		if (Item[i].SubMenu)
			Item[i].SubMenu->SetText(id,text);
		if (Item[i].ID==id){
			strcpy(Item[i].Name,text);
			ModifyMenu(hMenu,i,MF_STRING | MF_BYPOSITION,id,sys_str(text));
		}
	}
}



#endif
