#include "hui.h"
#ifdef HUI_API_WIN

namespace hui
{


HuiToolbar::HuiToolbar(HuiWindow *_win, bool vertical)
{
}

HuiToolbar::~HuiToolbar()
{
}

void HuiToolbar::Enable(bool _enabled)
{
}

#if 0

void CHuiWindow::EnableToolBar(bool enabled)
{
	if (tb!=&tool_bar[HuiToolBarTop])
		return;
	if (enabled)
		ShowWindow(tb->hWnd, SW_SHOW);
	else
		ShowWindow(tb->hWnd, SW_HIDE);

	if (enabled){
		RECT ToolBarRect;
		SendMessage(tool_bar[HuiToolBarTop].hWnd,TB_AUTOSIZE,0,0);
		GetWindowRect(tool_bar[HuiToolBarTop].hWnd,&ToolBarRect);
		int tbh=ToolBarRect.bottom-ToolBarRect.top;
		cdy+=tbh;
	}
	tb->Enabled=enabled;
}

void CHuiWindow::ToolBarSetCurrent(int index)
{
	index=HuiToolBarTop; // ... m(-_-)m
	tb=&tool_bar[index];
}

void CHuiWindow::ToolBarConfigure(bool text_enabled,bool large_icons)
{
	//SendMessage(tb->hWnd,TB_SETBUTTONSIZE,0,MAKELPARAM(80,30);
	SendMessage(tb->hWnd,TB_SETMAXTEXTROWS,(WPARAM)(text_enabled?1:0),0);

	
	/*TBADDBITMAP bitid;
	bitid.hInst = HINST_COMMCTRL;
	bitid.nID = large_icons?IDB_STD_LARGE_COLOR:IDB_STD_SMALL_COLOR;
	SendMessage(tb->hWnd, TB_ADDBITMAP, 1, (long)&bitid);
	SendMessage(tb->hWnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);*/
	tb->TextEnabled=text_enabled;
	tb->LargeIcons=large_icons;
}

static HWND _win_cur_hwnd_;
static char win_image_buffer[1048576];
int win_get_tb_image_id(sHuiToolBar *tb,int image)
{
	CHECKJPEGFORMAT;
	if (image>=1024){
		HBITMAP hbmp;
		int size=(tb->LargeIcons?24:16);
		if (strstr(hui_image_file[image-1024].c_str(),".png")){
			msg_todo("Windows support for png images... trying fallback to bmp");
			char filename[256];
			strcpy(filename,hui_image_file[image-1024].c_str());
			strcpy(strstr(filename,".png"),".bmp");
			//msg_write(filename);
			return win_get_tb_image_id(tb,HuiLoadImage(filename));
		}else if (strstr(hui_image_file[image-1024].c_str(),".ico")){
			HICON hIcon;
			//msg_write("ico  :(");
			hIcon = (HICON)LoadImage(	NULL,sys_str_f(hui_image_file[image-1024].c_str()),
										IMAGE_ICON,
										size,size,LR_LOADFROMFILE);
			//msg_write((int)hbmp);

			ICONINFO iconinfo;

			//hIcon = LoadIcon(NULL, sys_str_f(hui_image_file[image-1024].c_str()));
			if (hIcon==NULL){
				msg_error("LoadImage ico");
				msg_write(GetLastError());
			}
			//msg_write((int)hIcon);
			GetIconInfo(hIcon, &iconinfo);
			hbmp = iconinfo.hbmColor;
			//msg_write((int)hbmp);

		}else /*if (strstr(hui_image_file[image-1024].c_str(),".bmp"))*/{
			hbmp = (HBITMAP)LoadImage(	NULL,sys_str_f(hui_image_file[image-1024].c_str()),
										IMAGE_BITMAP,
										size,size,LR_LOADFROMFILE);//|LR_LOADTRANSPARENT);
		}
		TBADDBITMAP bitid;
		bitid.hInst = NULL;
		bitid.nID = (UINT)hbmp;
		int r=SendMessage(tb->hWnd, TB_ADDBITMAP, 1, (long)&bitid);
		//msg_write(r);
		return r;
	}
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

// just a helper function
void AddToolBarItem(sHuiToolBar *tb,int id,int type,CHuiMenu *menu)
{
	sHuiToolBarItem i;
	i.ID = id;
	i.Kind = type;
	i.Enabled = true;
	i.menu = menu;
	tb->Item.push_back(i);
}

// add a default button
void CHuiWindow::ToolBarAddItem(const char *title,const char *tool_tip,int image,int id)
{
	if (tb!=&tool_bar[HuiToolBarTop])
		return;
	TBBUTTON tbb;
	ZeroMemory(&tbb,sizeof(tbb));
	_win_cur_hwnd_=hWnd;
	tbb.iBitmap = win_get_tb_image_id(tb,image);
	tbb.fsState = TBSTATE_ENABLED;
	tbb.fsStyle = TBSTYLE_BUTTON;
	tbb.iString = (int)sys_str(title);
	tbb.idCommand = id;
	SendMessage(tb->hWnd, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
	AddToolBarItem(tb,id,HuiToolButton,NULL);
}

// add a checkable button
void CHuiWindow::ToolBarAddItemCheckable(const char *title,const char *tool_tip,int image,int id)
{
	if (tb!=&tool_bar[HuiToolBarTop])
		return;
	TBBUTTON tbb;
	ZeroMemory(&tbb,sizeof(tbb));
	_win_cur_hwnd_=hWnd;
	tbb.iBitmap = win_get_tb_image_id(tb,image);
	tbb.fsState = TBSTATE_ENABLED;
	tbb.fsStyle = TBSTYLE_CHECK;
	tbb.iString = (int)sys_str(title);
	tbb.idCommand = id;
	SendMessage(tb->hWnd, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
	AddToolBarItem(tb,id,HuiToolCheckable,NULL);
}

void CHuiWindow::ToolBarAddItemMenu(const char *title,const char *tool_tip,int image,CHuiMenu *menu,int id)
{
	if (!menu)
		return;
	if (tb!=&tool_bar[HuiToolBarTop])
		return;
	ToolBarAddItem(title,tool_tip,image,id);
	return;
	AddToolBarItem(tb,id,HuiToolMenu,menu);
}

// add a menu to the toolbar by resource id
void CHuiWindow::ToolBarAddItemMenuByID(const char *title,const char *tool_tip,int image,int menu_id,int id)
{
	CHuiMenu *menu=HuiCreateResourceMenu(menu_id);
	ToolBarAddItemMenu(title,tool_tip,image,menu,id);
}

void CHuiWindow::ToolBarAddSeparator()
{
	if (tb!=&tool_bar[HuiToolBarTop])
		return;
	TBBUTTON tbb;
	ZeroMemory(&tbb,sizeof(tbb));
	tbb.fsStyle = TBSTYLE_SEP;
	SendMessage(tb->hWnd, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
}

// remove all items from the toolbar
void CHuiWindow::ToolBarReset()
{
	if (tb!=&tool_bar[HuiToolBarTop])
		return;
	for (int i=0;i<tb->Item.size();i++)
		SendMessage(tb->hWnd,(UINT)TB_DELETEBUTTON,(WPARAM)0/*i_button*/,(LPARAM)0);
	tb->Item.clear();
}


void CHuiWindow::_ToolBarEnable_(int id, bool enabled)
{
	allow_signal_level++;
	allow_signal_level--;
}

void CHuiWindow::_ToolBarCheck_(int id, bool checked)
{
	allow_signal_level++;
	allow_signal_level--;
}

bool CHuiWindow::_ToolBarIsChecked_(int id)
{
	return false;
}

#endif

void HuiToolbar::add(HuiControl *c)
{
}

};

#endif
