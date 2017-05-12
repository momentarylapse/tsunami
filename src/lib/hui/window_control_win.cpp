#include "hui.h"

namespace hui
{


#ifdef HUI_API_WIN


void HuiWindow::SetMenu(HuiMenu *m){}
void HuiWindow::SetTitle(const string &title){}
void HuiWindow::SetPosition(int x, int y){}
void HuiWindow::SetSize(int w, int h){}
irect HuiWindow::GetOuterior()
{
	return irect(0,0,0,0);
}
string HuiWindow::Run()
{
	return "";
}

void HuiWindow::AddButton(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddDefButton(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddText(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddGroup(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddComboBox(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddEdit(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddMultilineEdit(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddCheckBox(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddRadioButton(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddListView(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddIconView(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddTabControl(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddTreeView(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddControlTable(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddColorButton(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddToggleButton(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddSpinButton(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddDrawingArea(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddImage(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddSlider(const string &title, int x, int y, int width, int height, const string &id){}
void HuiWindow::AddProgressBar(const string &title, int x, int y, int width, int height, const string &id){}

void HuiWindow::SetTarget(const string &id, int page){}

HuiWindow::~HuiWindow(){}

void HuiWindow::_Init_(const string &title,int x, int y, int w, int h, HuiWindow *parent, bool allow_parent, int mode){}

#if 0

void GetPartStrings(int id, const char *title);
const char *ScanOptions(int id, const char *title);
extern int NumPartStrings;
extern char PartString[64][512], OptionString[512];
extern bool hui_option_tree;
extern bool hui_option_icons;
extern bool hui_option_extended;
extern bool hui_option_multiline;
extern bool hui_option_alpha;
extern bool hui_option_bold;
extern bool hui_option_italic;
extern bool hui_option_nobar;

//----------------------------------------------------------------------------------
// creating control items



// general control...just a helper function, don't use!!!
void AddControl(HuiWindow *win,sHuiControl *c,int id,int kind)
{
	if (c->hWnd)
		SendMessage(c->hWnd,(UINT)WM_SETFONT,(WPARAM)hui_win_default_font,(LPARAM)TRUE);
	/*if (win->NumControl.size()==0)
		SetFocus(c);*/
	if (kind!=HuiKindColorButton)
		c->hWnd3=NULL;
	
	c->ID=id;
	c->Kind=kind;
	c->Enabled=true;
	c->TabID=win->TabCreationID;
	c->TabPage=win->TabCreationPage;
	/*c->x=0;
	c->y=0;*/
	win->Control.push_back(*c);
}

void HuiWindow::AddButton(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	c.hWnd=CreateWindow(	_T("BUTTON"),get_lang_sys(id,title),
							WS_VISIBLE | WS_CHILD | (HuiUseFlatButtons?BS_FLAT:0),
							x+cdx,y+cdy,width,height,
							hWnd,NULL,hui_win_instance,NULL);
	AddControl(this,&c,id,HuiKindButton);
}

void HuiWindow::AddColorButton(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	ScanOptions(id, title);
	int bw=(width>25)?25:width;
	if (hui_option_alpha){
		bw=(width>50)?50:width;
		c.hWnd=CreateWindow(_T("BUTTON"),_T("rgb"),
							WS_VISIBLE | WS_CHILD | (HuiUseFlatButtons?BS_FLAT:0),
							x+width-bw+cdx,y+cdy,bw/2,height,
							hWnd,NULL,hui_win_instance,NULL);
		c.hWnd3=CreateWindow(	_T("BUTTON"),_T("a"),
								WS_VISIBLE | WS_CHILD | (HuiUseFlatButtons?BS_FLAT:0),
								x+width-bw/2+cdx,y+cdy,bw/2,height,
								hWnd,NULL,hui_win_instance,NULL);
	}else{
		c.hWnd=CreateWindow(_T("BUTTON"),_T("rgb"),
							WS_VISIBLE | WS_CHILD | (HuiUseFlatButtons?BS_FLAT:0),
							x+width-bw+cdx,y+cdy,bw,height,
							hWnd,NULL,hui_win_instance,NULL);
		c.hWnd3=NULL;
	}
	c.hWnd2=CreateWindow(	_T("STATIC"),_T(""),//get_lang_sys(id,title),
							WS_CHILD | WS_VISIBLE,
							x+cdx,y+cdy,width-bw,height,
							hWnd,NULL,hui_win_instance,NULL);
	c.Color[0]=0;
	c.Color[1]=0;
	c.Color[2]=0;
	c.Color[3]=255;
	AddControl(this,&c,id,HuiKindColorButton);
}

void HuiWindow::AddDefButton(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	c.hWnd=CreateWindow(_T("BUTTON"),get_lang_sys(id,title),
						WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | (HuiUseFlatButtons?BS_FLAT:0),
						x+cdx,y+cdy,width,height,
						hWnd,NULL,hui_win_instance,NULL);
	AddControl(this,&c,id,HuiKindButton);
}

void HuiWindow::AddCheckBox(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	c.hWnd=CreateWindow(_T("BUTTON"),get_lang_sys(id,title),
						WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX| (HuiUseFlatButtons?BS_FLAT:0),
						x+cdx,y+cdy,width,height,
						hWnd,NULL,hui_win_instance,NULL);
	AddControl(this,&c,id,HuiKindCheckBox);
}

void HuiWindow::AddText(const char *title,int x,int y,int width,int height,int id)
{
	const char *title2 = ScanOptions(id, title);
	sHuiControl c;
	c.hWnd=CreateWindow(_T("STATIC"),sys_str(title2),
						WS_CHILD | WS_VISIBLE,
						x+cdx,y+cdy,width,height,
						hWnd,NULL,hui_win_instance,NULL);
	AddControl(this,&c,id,HuiKindText);
}

void HuiWindow::AddEdit(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	if (HuiUseFlatButtons){
		c.hWnd=CreateWindow(_T("EDIT"),sys_str(title),//get_lang_sys(id,title),
							WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER | (HuiMultiline?(ES_MULTILINE | ES_AUTOVSCROLL):0),
							x+cdx,y+cdy+2,width,height-4,
							hWnd,NULL,hui_win_instance,NULL);
	}else{
		c.hWnd=CreateWindowEx(	WS_EX_CLIENTEDGE,_T("EDIT"),sys_str(title),//get_lang_sys(id,title),
								//WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
								WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | (HuiMultiline?(ES_MULTILINE | ES_AUTOVSCROLL):0),// | ES_PASSWORD | ES_NUMBER,
								x+cdx,y+cdy,width,height,
								hWnd,NULL,hui_win_instance,NULL);
	}
	AddControl(this,&c,id,HuiKindEdit);

	if ((height>30)&&(!HuiMultiline)){
		HFONT f=CreateFont(height,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("MS Sans Serif"));
		//WinStandartFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

		SendMessage(Control.back().hWnd,(UINT)WM_SETFONT,(WPARAM)f,(LPARAM)TRUE);
	}
}

void HuiWindow::AddGroup(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	c.hWnd=CreateWindow(_T("BUTTON"),get_lang_sys(id,title),
						WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
						x+cdx,y+cdy,width,height,
						hWnd,NULL,hui_win_instance,NULL);
	AddControl(this,&c,id,HuiKindGroup);
}

void HuiWindow::AddComboBox(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	c.hWnd=CreateWindow(_T("COMBOBOX"),get_lang_sys(id,title),
						WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | CBS_DISABLENOSCROLL,
						x+cdx,y+cdy+2,width,/*height*/500,
						hWnd,NULL,hui_win_instance,NULL);
	SendMessage(c.hWnd,(UINT)CB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
	AddControl(this,&c,id,HuiKindComboBox);
	GetPartStrings(id,title);
	for (int i=0;i<NumPartStrings;i++)
		AddString(id, PartString[i]);
	SetInt(id, 0);
}

void HuiWindow::AddTabControl(const char *title,int x,int y,int width,int height,int id)
{
	//const char *title2 = ScanOptions(id, title);
	sHuiControl c;
	if (HuiUseFlatButtons){
		// tabcontrol itself
		c.hWnd=CreateWindow(_T("SysTabControl32"),_T(""),
							WS_CHILD | WS_VISIBLE,// | TCS_BUTTONS | TCS_FLATBUTTONS,
							x+cdx,y+cdy,width,height,
							hWnd,NULL,hui_win_instance,NULL);
		/*c.hWnd2=CreateWindow(_T("BUTTON"),_T(""),
												WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
												x+cdx,y+cdy+16,width,height-16,
												hWnd,NULL,HuihInstance,NULL);*/
		// rectangle
		/*c.hWnd2=CreateWindow(_T("STATIC"),_T(""),
												WS_CHILD | WS_VISIBLE | SS_SUNKEN,
												x+cdx,y+cdy+24,width,height-24,
												hWnd,NULL,hui_win_instance,NULL);*/
	}else{
		c.hWnd=CreateWindow(	_T("SysTabControl32"),_T(""),
												WS_CHILD | WS_VISIBLE,
												x+cdx,y+cdy,width,height,
												hWnd,NULL,hui_win_instance,NULL);
	}
	GetPartStrings(id,title);
	for (int i=0;i<NumPartStrings;i++){
		TCITEM item;
		item.mask=TCIF_TEXT;
		item.pszText=(win_str)sys_str(PartString[i]);//const_cast<LPSTR>(PartString[i]);
		//msg_write(PartString[i]);
		TabCtrl_InsertItem(c.hWnd,i,&item);
	}
	c.x=x+cdx;
	c.y=y+cdy;
	//SetControlSelection(id,0);
	AddControl(this,&c,id,HuiKindTabControl);
}

void HuiWindow::SetTabCreationPage(int id,int page)
{
	TabCreationID=id;
	TabCreationPage=page;
	cdx=0;
	cdy=0;
	if (tool_bar[HuiToolBarTop].Enabled){
		RECT ToolBarRect;
		SendMessage(tool_bar[HuiToolBarTop].hWnd,TB_AUTOSIZE,0,0);
		GetWindowRect(tool_bar[HuiToolBarTop].hWnd,&ToolBarRect);
		int tbh=ToolBarRect.bottom-ToolBarRect.top;
		cdy=tbh;
	}
	if (id >= 0)
		for (int i=0;i<Control.size();i++)
			if (id==Control[i].ID){
				RECT r;
				memset(&r,0,sizeof(r));
				TabCtrl_AdjustRect(Control[i].hWnd,FALSE,&r);
				cdx=Control[i].x+r.left-4;
				cdy=Control[i].y+r.top-2;
			}
}

enum
{
   TITLE_COLUMN,
   AUTHOR_COLUMN,
   CHECKED_COLUMN,
   N_COLUMNS
};


void HuiWindow::AddListView(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	GetPartStrings(id,title);

	if (hui_option_icons){
	}else if (hui_option_tree){
		DWORD style=WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS;
		c.hWnd=CreateWindowEx((HuiUseFlatButtons?0:WS_EX_CLIENTEDGE),WC_TREEVIEW,_T(""),
												style | (HuiUseFlatButtons?( BS_FLAT ):0),
												x+cdx,y+cdy,width,height,
												hWnd,NULL,hui_win_instance,NULL);
	}else{
		DWORD style=WS_CHILD | WS_VISIBLE | LVS_REPORT | (HuiMultiline?LVS_SHOWSELALWAYS:LVS_SINGLESEL);
		c.hWnd=CreateWindowEx((HuiUseFlatButtons?0:WS_EX_CLIENTEDGE),WC_LISTVIEW,_T(""),
												style | (HuiUseFlatButtons?( WS_BORDER | BS_FLAT ):0),
												x+cdx,y+cdy,width,height,
												hWnd,NULL,hui_win_instance,NULL);

		for (int i=0;i<NumPartStrings;i++){
			LVCOLUMN lvc;
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;
			lvc.iSubItem = i;//num_cols;
			lvc.pszText = (win_str)sys_str(PartString[i]);
			lvc.cx = 100;
			ListView_InsertColumn(c.hWnd,i,&lvc);
		}
		ListView_SetExtendedListViewStyleEx(c.hWnd,0,LVS_EX_FULLROWSELECT /*| LVS_EX_GRIDLINES*/ | LVS_EX_HEADERDRAGDROP | (HuiUseFlatButtons?LVS_EX_FLATSB:0));
		//ListView_SetExtendedListViewStyleEx(c.hWnd,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | (HuiUseFlatButtons?LVS_EX_FLATSB:0));
		for (int i=0;i<NumPartStrings;i++)
			ListView_SetColumnWidth(c.hWnd,i,LVSCW_AUTOSIZE_USEHEADER);
	}
	AddControl(this, &c, id, hui_option_icons ? HuiKindListViewIcons : (hui_option_tree ? HuiKindListViewTree : HuiKindListView));
}

void HuiWindow::AddProgressBar(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	c.hWnd=CreateWindowEx(0,PROGRESS_CLASS,_T("test"),
											WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
											x+cdx,y+cdy,width,height,
											hWnd,NULL,hui_win_instance,NULL);
	// set range to (int)0 - (int)65535 ....   "int"  (-_-')
	//SendMessage(c.hWnd, PBM_SETRANGE, 0, MAKEPARAM(0,65535));
//	SendMessage(c.hWnd, PBM_SETRANGE, 0, 65535);
	AddControl(this,&c,id,HuiKindProgressBar);
}

void HuiWindow::AddSlider(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	//???
	c.hWnd=CreateWindowEx(0,PROGRESS_CLASS,_T("test"),
											WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
											x+cdx,y+cdy,width,height,
											hWnd,NULL,hui_win_instance,NULL);
	// set range to (int)0 - (int)65535 ....   "int"  (-_-')
	//SendMessage(c.hWnd, PBM_SETRANGE, 0, MAKEPARAM(0,65535));
//	SendMessage(c.hWnd, PBM_SETRANGE, 0, 65535);
	AddControl(this,&c,id,HuiKindSlider);
}

void HuiWindow::AddImage(const char *title,int x,int y,int width,int height,int id)
{
	sHuiControl c;
	c.hWnd=CreateWindow(	_T("STATIC"),_T(""),
											WS_CHILD | WS_VISIBLE | SS_BITMAP,
											x+cdx,y+cdy,width,height,
											hWnd,NULL,hui_win_instance,NULL);
	HBITMAP bmpname = (HBITMAP)LoadImage(NULL, sys_str_f(title), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	SendMessage(c.hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmpname);
	AddControl(this,&c,id,HuiKindImage);
}

void HuiWindow::AddDrawingArea(const char *title,int x,int y,int width,int height,int id)
{
	msg_todo("Hui: AddDrawingArea (Windows)");
	return;
}



//----------------------------------------------------------------------------------
// drawing

void HuiWindow::Redraw(int id)
{
}

void HuiWindow::BeginDraw(int id)
{
}

void HuiWindow::EndDraw()
{
}

void HuiWindow::SetDrawingColor(float r, float g, float b)
{
}

void HuiWindow::SetDrawingColor2(const color &c)
{
}

void HuiWindow::SetLineWidth(float w)
{
}

void HuiWindow::DrawPoint(float x, float y)
{
}

void HuiWindow::DrawLine(float x1, float y1, float x2, float y2)
{
}

void HuiWindow::DrawLines(float *x, float *y, int num_lines)
{
}

void HuiWindow::DrawLinesMA(Array<float> &x, Array<float> &y)
{
	DrawLines(&x[0], &y[0], x.num - 1);
}

void HuiWindow::DrawStr(float x, float y, const char *str)
{
}

void HuiWindow::DrawRect(float x1, float y1, float x2, float y2)
{
}




//----------------------------------------------------------------------------------
// data exchanging functions for control items



// replace all the text
//    for all
void HuiWindow::SetString(int id, const char *str)
{
	allow_signal_level++;
	//char *str2=sys_str(str);
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			if ((Control[i].Kind==HuiKindEdit)||(Control[i].Kind==HuiKindCheckBox)||(Control[i].Kind==HuiKindButton)||(Control[i].Kind==HuiKindText)||(Control[i].Kind==HuiKindGroup))
				SendMessage(Control[i].hWnd,WM_SETTEXT,(WPARAM)0,(LPARAM)sys_str(str));
			else if (Control[i].Kind==HuiKindComboBox)
				SendMessage(Control[i].hWnd,CB_ADDSTRING,(WPARAM)0,(LPARAM)sys_str(str));
			else if ((Control[i].Kind==HuiKindListView)||(Control[i].Kind==HuiKindListViewTree)){
				AddString(id,str);
			}
		}
	if (ID==id)
		SetTitle(str);
	allow_signal_level--;
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void HuiWindow::SetInt(int id, int n)
{
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			if ((Control[i].Kind==HuiKindEdit) || (Control[i].Kind==HuiKindText))
				SetString(id, i2s(n));
			if (Control[i].Kind==HuiKindTabControl){
				TabCtrl_SetCurSel(Control[i].hWnd,n);
				UpdateTabPages(this);
			}else if (Control[i].Kind==HuiKindListView)
				SendMessage(Control[i].hWnd,(UINT)LVM_SETSELECTIONMARK,(WPARAM)n,(LPARAM)0);
			else if (Control[i].Kind==HuiKindComboBox)
				SendMessage(Control[i].hWnd,(UINT)CB_SETCURSEL,(WPARAM)n,(LPARAM)0);
		}
	allow_signal_level--;
}

// replace all the text with a float
//    for all
void HuiWindow::SetFloat(int id, float f)
{
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			if (Control[i].Kind==HuiKindProgressBar)
				SendMessage(Control[i].hWnd, PBM_SETPOS, int(100.0f*f), 0);
			else
				SetString(id, f2s(f, NumFloatDecimals));
		}
	allow_signal_level--;
}

void HuiWindow::SetImage(int id,int image)
{
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			// TODO
		}
	allow_signal_level--;
}

static LVCOLUMN _col_;
static LVITEM _lvI_;
static TVITEM _tvi_;

// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void HuiWindow::AddString(int id, const char *str)
{
	allow_signal_level++;
	//char *str2=sys_str(str);
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			if ((Control[i].Kind==HuiKindEdit)||(Control[i].Kind==HuiKindCheckBox)||(Control[i].Kind==HuiKindButton)||(Control[i].Kind==HuiKindText)||(Control[i].Kind==HuiKindGroup))
				SendMessage(Control[i].hWnd,WM_SETTEXT,(WPARAM)0,(LPARAM)sys_str(str));
			else if (Control[i].Kind==HuiKindComboBox)
				SendMessage(Control[i].hWnd,CB_ADDSTRING,(WPARAM)0,(LPARAM)sys_str(str));
			else if (Control[i].Kind==HuiKindListView){
				int line=ListView_GetItemCount(Control[i].hWnd),j;
				GetPartStrings(-1,str);
				for (j=0;j<NumPartStrings;j++){
					if (j==0){
						_lvI_.iItem = line;
						_lvI_.iSubItem = 0;
						_lvI_.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
						_lvI_.state = 0;
						_lvI_.stateMask = 0;
						_lvI_.pszText = (win_str)sys_str(PartString[j]);
						ListView_InsertItem(Control[i].hWnd, &_lvI_);
					}else
						ListView_SetItemText(Control[i].hWnd,line,j,(win_str)sys_str(PartString[j]));
				}
				if ((line<5)||(NumPartStrings==0)){
					for (j=0;j<128;j++)
						if (ListView_GetColumn(Control[i].hWnd,j,&_col_)){
							if (!ListView_SetColumnWidth(Control[i].hWnd,j,LVSCW_AUTOSIZE_USEHEADER))
								break;
						}else
							break;
					ShowWindow(Control[i].hWnd,SW_HIDE);
					ShowWindow(Control[i].hWnd,SW_SHOW);
				}
			}else if (Control[i].Kind==HuiKindListViewTree){
				GetPartStrings(-1,str);
				char tt[1024];
				strcpy(tt, PartString[0]);
				for (int j=1;j<NumPartStrings;j++)
					strcat(tt, string(" - ", PartString[j]));
				_tvi_.mask = TVIF_TEXT;
				_tvi_.pszText = (win_str)sys_str(tt);
				_tvi_.cchTextMax = sizeof(_tvi_.pszText)/sizeof(_tvi_.pszText[0]);

				TVINSERTSTRUCT tvins;
				tvins.item = _tvi_;
				tvins.hInsertAfter = TVI_ROOT;
				tvins.hParent = TVI_ROOT;
				Control[i]._item_.push_back((HWND)SendMessage(Control[i].hWnd,TVM_INSERTITEM,0,(LPARAM)&tvins));
			}
		}
	allow_signal_level--;
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void HuiWindow::AddChildString(int id, int parent_row, const char *str)
{
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindListViewTree){
				GetPartStrings(-1,str);
				char tt[1024];
				strcpy(tt, PartString[0]);
				for (int j=1;j<NumPartStrings;j++)
					strcat(tt, string(" - ", PartString[j]));
				TVITEM tvi;
				tvi.mask = TVIF_TEXT;
				tvi.pszText = (win_str)sys_str(tt);
				tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]);

				TVINSERTSTRUCT tvins;
				tvins.item = tvi;
				tvins.hInsertAfter = (HTREEITEM)Control[i]._item_.back();
				tvins.hParent = (HTREEITEM)Control[i]._item_[parent_row];
				Control[i]._item_.push_back((HWND)SendMessage(Control[i].hWnd,TVM_INSERTITEM,0,(LPARAM)&tvins));
			}
	allow_signal_level--;
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void HuiWindow::ChangeString(int id,int row,const char *str)
{
	allow_signal_level++;
	// TODO
	allow_signal_level--;
}

void HuiWindow::SetColor(int id,int *c,bool use_alpha)
{
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindColorButton){
				for (int j=0;j<(use_alpha?4:3);j++)
					Control[i].Color[j]=c[j];
				// redraw color field...
				ShowWindow(Control[i].hWnd2,SW_HIDE);
				ShowWindow(Control[i].hWnd2,SW_SHOW);
			}
	allow_signal_level--;
}

static char ControlText[2048];//,ControlLine[2048];

// retrieve the text
//    for edit
const char *HuiWindow::GetString(int id)
{
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindEdit){
				/*strcpy(ControlText,"");
				int nl=SendMessage(Control[i].hWnd,(UINT)EM_GETLINECOUNT,(WPARAM)0,(LPARAM)0);
				for (int j=0;j<nl;j++){
					ControlLine[0]=0; // 2048 bytes Groesse...
					ControlLine[1]=8;
					SendMessage(Control[i].hWnd,(UINT)EM_GETLINE,(WPARAM)j,(LPARAM)ControlLine);
					strcat(ControlText,ControlLine);
					if (j<nl-1)
						strcat(ControlText,"\n");
				}*/
				TCHAR _temp_[2048];
				ZeroMemory(_temp_,sizeof(_temp_));
				SendMessage(Control[i].hWnd,(UINT)WM_GETTEXT,(WPARAM)2048,(LPARAM)_temp_);//ControlText);
				return de_sys_str(_temp_);//DeSysStr(ControlText);
			}
	strcpy(ControlText,"");
	return ControlText;
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int HuiWindow::GetInt(int id)
{
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			if ((Control[i].Kind==HuiKindEdit) || (Control[i].Kind==HuiKindText))
				return s2i(GetString(id));
			if (Control[i].Kind==HuiKindTabControl)
				return TabCtrl_GetCurSel(Control[i].hWnd);
			else if (Control[i].Kind==HuiKindListView)
				return ListView_GetSelectionMark(Control[i].hWnd);
			else if (Control[i].Kind==HuiKindComboBox)
				return (int)SendMessage(Control[i].hWnd,(UINT)/*CB_GETCOUNT*/CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
		}
	return -1;
}

// retrieve the text as a numerical value (float)
//    for edit
float HuiWindow::GetFloat(int id)
{
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindSlider){
				// todo...
			}
	return s2f(GetString(id));
}

void HuiWindow::GetColor(int id,int *c,bool use_alpha)
{
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindColorButton){
				for (int j=0;j<(use_alpha?4:3);j++)
					c[j]=Control[i].Color[j];
			}
}

// switch control to usable/unusable
//    for all
void HuiWindow::Enable(int id,bool enabled)
{
	if (id<0)
		return;
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
		    Control[i].Enabled=enabled;
			EnableWindow(Control[i].hWnd,enabled);
			if (Control[i].Kind==HuiKindColorButton){
				if (Control[i].hWnd3)
					EnableWindow(Control[i].hWnd3,enabled);
				ShowWindow(Control[i].hWnd2,SW_HIDE);
				ShowWindow(Control[i].hWnd2,SW_SHOW);
			}
		}
	for (int t=0;t<4;t++)
		for (int i=0;i<tool_bar[t].Item.size();i++)
			if (id==tool_bar[t].Item[i].ID){
			    tool_bar[t].Item[i].Enabled=enabled;
				if (t==HuiToolBarTop)
					SendMessage(tool_bar[t].hWnd,TB_ENABLEBUTTON,(WPARAM)id,(LPARAM)(enabled?TRUE:FALSE));
			}
	allow_signal_level--;
}

// show/hide control
//    for all
void HuiWindow::HideControl(int id,bool hide)
{
	if (id<0)
		return;
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			if (hide)
				ShowWindow(Control[i].hWnd,SW_HIDE);
			else
				ShowWindow(Control[i].hWnd,SW_SHOW);
			// TESTME!
		}
	allow_signal_level--;
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void HuiWindow::Check(int id,bool checked)
{
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindCheckBox)
				SendMessage(Control[i].hWnd,BM_SETCHECK,(WPARAM)(checked?BST_CHECKED:BST_UNCHECKED),(LPARAM)0);
	// BST_INDETERMINATE
	for (int t=0;t<4;t++)
		for (int i=0;i<tool_bar[t].Item.size();i++)
			if (id==tool_bar[t].Item[i].ID)
				if (tool_bar[t].Item[i].Kind==HuiToolCheckable){
					if (t==HuiToolBarTop)
						SendMessage(tool_bar[t].hWnd,TB_CHECKBUTTON,(WPARAM)id,(LPARAM)(checked?TRUE:FALSE));
				}
	allow_signal_level--;
}

// is marked as "checked"?
//    for CheckBox
bool HuiWindow::IsChecked(int id)
{
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindCheckBox)
				return SendMessage(Control[i].hWnd,BM_GETCHECK,(WPARAM)0,(LPARAM)0)==BST_CHECKED;
	
	for (int t=0;t<4;t++)
		for (int i=0;i<tool_bar[t].Item.size();i++)
			if (id==tool_bar[t].Item[i].ID)
				if (tool_bar[t].Item[i].Kind==HuiToolCheckable){
					if (t==HuiToolBarTop)
						return (SendMessage(tool_bar[t].hWnd,TB_ISBUTTONCHECKED,(WPARAM)i,(LPARAM)0)!=0);
				}
	return false;
}

// which lines are selected?
//    for ListView
Array<int> HuiWindow::GetMultiSelection(int id)
{
	Array<int> sel;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID)
			if (Control[i].Kind==HuiKindListView){
				int num_lines=ListView_GetItemCount(Control[i].hWnd);

				for (int j=0;j<num_lines;j++){
					LVITEM lvI;
   					lvI.iItem = j;
					lvI.iSubItem = 0;
					lvI.mask = LVIF_STATE;
					lvI.state = 0;
					lvI.stateMask = LVIS_SELECTED;
					ListView_GetItem(Control[i].hWnd,&lvI);
					if (lvI.state>0){
						sel.add(j);
					}
				}
			}
	return sel;
}

// delete all the content
//    for ComboBox, ListView
void HuiWindow::Reset(int id)
{
	allow_signal_level++;
	for (int i=0;i<Control.size();i++)
		if (id==Control[i].ID){
			if (Control[i].Kind==HuiKindListView)
				ListView_DeleteAllItems(Control[i].hWnd);
			else if (Control[i].Kind==HuiKindComboBox)
				SendMessage(Control[i].hWnd,CB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
		}
	allow_signal_level--;
}

#endif

#endif

}
