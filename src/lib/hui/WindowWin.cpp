#include "hui.h"
#ifdef HUI_API_WIN

#include <windows.h>

namespace hui
{

void HuiWindow::__delete__()
{
	this->HuiWindow::~HuiWindow();
}

#if 0

//----------------------------------------------------------------------------------
// window message handling

// find a toolbar item
bool TestToolBarID(HuiWindow *win,int id,message_function *mf)
{
	if (id<0)
		return false;
	for (int k=0;k<4;k++)
		if (win->tool_bar[k].Enabled)
			for (unsigned int i=0;i<win->tool_bar[k].Item.size();i++)
				if (win->tool_bar[k].Item[i].ID==id){
					if (win->ready)
						mf(id);
					return true;
				}
	return false;
}

static int win_reg_no=0;


void UpdateTabPages(HuiWindow *win)
{
	for (unsigned int i=0;i<win->Control.size();i++){
		int n_tab=-1;
		int cmd=SW_SHOW;
		// find the tab-control
	    if (win->Control[i].TabID>=0)
			for (unsigned int j=0;j<i;j++)
				if ((win->Control[j].Kind==HuiKindTabControl)&&(win->Control[j].ID==win->Control[i].TabID))
					n_tab=j;
	    if (n_tab>=0){
			cmd=SW_HIDE;
			// recursive: Tab-Control visible?    (n_tab < i !!!!!)
			if ((win->Control[i].TabPage==win->GetInt(win->Control[i].TabID))&& (IsWindowVisible(win->Control[n_tab].hWnd)))
			    cmd=SW_SHOW;
		}
		if (win->Control[i].hWnd2)//win->Control[i].Kind==HuiKindEdit)
			ShowWindow(win->Control[i].hWnd2,cmd);
		if (win->Control[i].hWnd3)
			ShowWindow(win->Control[i].hWnd3,cmd);
		ShowWindow(win->Control[i].hWnd,cmd);
	}
	//msg_write>Write("//Tab");
}

/*static void ExecuteWinMessageFunc(HuiWindow *win,int id)
{
	if (win){
		win->OwnDataOld=win->OwnData;
		for (i=0;i<256;i++)
			win->OwnData.key[KeyID[i]]=(GetAsyncKeyState(i)!=0);
		int k=-1;
		if ((!win->GetKey(KEY_RALT))&&(!win->GetKey(KEY_LALT))){
			for (i=6;i<NUM_KEYS;i++)
				if (win->GetKeyDown(i)){
					k=i;
					if ((win->GetKey(KEY_RCONTROL))||(win->GetKey(KEY_LCONTROL)))	k+=256;
					if ((win->GetKey(KEY_RSHIFT))||(win->GetKey(KEY_LSHIFT)))		k+=512;
				}
			if (k>=0)
				for (i=0;i<HuiNumKeyCodes;i++)
					if (k==HuiKeyCode[i])
						mf(HuiKeyCodeID[i]);
		}
	}
}*/

struct s_win_bitmap{
	BITMAPINFOHEADER header; 
	RGBQUAD color[64]; 
};

static s_win_bitmap _win_bitmap_;
static int win_temp_color[4];

void _win_color_interpolate_(int c[4],int c1[4],int c2[4],float t)
{
	c[0]=(int)( (float)c1[0] * (1-t) + (float)c2[0] * t );
	c[1]=(int)( (float)c1[1] * (1-t) + (float)c2[1] * t );
	c[2]=(int)( (float)c1[2] * (1-t) + (float)c2[2] * t );
}

RGBQUAD _win_color_from_i4_(int c[4])
{
	RGBQUAD wc;
	wc.rgbRed  =c[0];
	wc.rgbGreen=c[1];
	wc.rgbBlue =c[2];
	return wc;
}

static int _win_c_tbg1_[4]={255,255,255,0};
static int _win_c_tbg2_[4]={120,120,120,0};
static int _win_c_gray_[4]={127,127,127,0};

HRESULT _win_color_brush_(int c[4],bool enabled)
{
	memset(&_win_bitmap_.header,0,sizeof(_win_bitmap_.header));
	_win_bitmap_.header.biSize=sizeof(BITMAPINFOHEADER);
	_win_bitmap_.header.biHeight=8;
	_win_bitmap_.header.biWidth=8;
	_win_bitmap_.header.biBitCount=32;
	_win_bitmap_.header.biCompression=BI_RGB;
	_win_bitmap_.header.biPlanes=1;
	int c1[4],c2[4];
	_win_color_interpolate_(c1,_win_c_tbg1_,c,float(c[3])/255.0f);
	_win_color_interpolate_(c2,_win_c_tbg2_,c,float(c[3])/255.0f);
	if (!enabled){
		_win_c_gray_[0]=GetRValue(GetSysColor(COLOR_3DFACE));
		_win_c_gray_[1]=GetGValue(GetSysColor(COLOR_3DFACE));
		_win_c_gray_[2]=GetBValue(GetSysColor(COLOR_3DFACE));
		_win_color_interpolate_(c1,_win_c_gray_,c1,0.5f);
		_win_color_interpolate_(c2,_win_c_gray_,c2,0.5f);
	}
	RGBQUAD r1,r2;
	r1=_win_color_from_i4_(c1);
	r2=_win_color_from_i4_(c2);
	for (int x=0;x<8;x++)
		for (int y=0;y<8;y++){
			_win_bitmap_.color[y*8+x] = ( (x<4)^(y<4) ) ? r1 : r2;
		}
	return (LRESULT)CreateDIBPatternBrushPt((PBITMAPINFO)&_win_bitmap_,DIB_RGB_COLORS);
}

static bool win_proc_force_return;
static HBRUSH bkground;

static LRESULT WindowProcedureDefaultStuff(HuiWindow *win,HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	win_proc_force_return=true;

	//msg_write>Write(" w");
	win->CompleteWindowMessage.msg=message;
	win->CompleteWindowMessage.wparam=(unsigned int)wParam;
	win->CompleteWindowMessage.lparam=(unsigned int)lParam;

	// losing focus -> disable keyboard handling
	if (GetActiveWindow()!=win->hWnd){
		win->AllowKeys=false;
		win->OwnData.KeyBufferDepth=0;
	}

	//msg_write>Write("-");
	//msg_write>Write((int)message);
	//msg_write>Write((int)(GetActiveWindow()==win->hWnd));


	// Nix input handling
	if (win->UsedByNix)
		if (win->NixGetInputFromWindow)
			win->NixGetInputFromWindow();

	switch (message){
		case WM_COMMAND:
			if (HIWORD(wParam)==BN_CLICKED){
				for (unsigned int i=0;i<win->Control.size();i++)
					if (win->Control[i].Kind==HuiKindColorButton){
						if (win->Control[i].hWnd==(HWND)lParam)
							// color button clicked -> start color selector
							if (HuiSelectColor(win,win->Control[i].Color[0],win->Control[i].Color[1],win->Control[i].Color[2])){
								memcpy(win->Control[i].Color,HuiColor,12);
								//SendMessage(win->Control[i].hWnd2,(UINT),0,0);
								ShowWindow(win->Control[i].hWnd2,SW_HIDE);
								ShowWindow(win->Control[i].hWnd2,SW_SHOW);
							}
						if (win->Control[i].hWnd3==(HWND)lParam)
							// color button clicked -> start color selector     (alpha... ugly, but ok...)
							if (HuiSelectColor(win,win->Control[i].Color[3],win->Control[i].Color[3],win->Control[i].Color[3])){
								win->Control[i].Color[3]=(HuiColor[0]+HuiColor[1]+HuiColor[2])/3;
								//SendMessage(win->Control[i].hWnd2,(UINT),0,0);
								ShowWindow(win->Control[i].hWnd2,SW_HIDE);
								ShowWindow(win->Control[i].hWnd2,SW_SHOW);
							}
					}
			}
			break;
			
		case WM_CTLCOLORSTATIC:
			// setting background for color button
			for (int i=0;i<win->Control.size();i++)
				if (win->Control[i].Kind==HuiKindColorButton)
					if (win->Control[i].hWnd2==(HWND)lParam){
						//return (LRESULT)CreateSolidBrush(RGB(win->Control[i].Color[0],win->Control[i].Color[1],win->Control[i].Color[2]));
						return _win_color_brush_(win->Control[i].Color,win->Control[i].Enabled);
					}
			// default color...
			SetBkColor ((HDC) wParam, GetSysColor(COLOR_3DFACE));
			bkground = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			for (int i=0;i<win->Control.size();i++)
				if (win->Control[i].hWnd==(HWND)lParam)
					if (win->Control[i].TabID>=0){
						SetBkColor ((HDC) wParam, GetSysColor(COLOR_BTNHIGHLIGHT));
						bkground = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
					}
			return((DWORD) bkground);
			//return DefWindowProc(hwnd,message,wParam,lParam);
	}

	// disabled windows (having terror children) -> default...
	if (!win->Allowed){
		//msg_write>Write(string2("!win->Allowed %d",message));
		switch (message){
			case WM_PAINT:
				DefWindowProc(hwnd,message,wParam,lParam);
				if (win->MessageFunction)
					win->MessageFunction(HUI_WIN_RENDER);
				break;
			case WM_ERASEBKGND:
				DefWindowProc(hwnd,message,wParam,lParam);
				//if (win->MessageFunction)
				//	win->MessageFunction(HUI_WIN_ERASEBKGND);
				break;
			case WM_SETFOCUS:
				if (win->TerrorChild)
					SetFocus(win->TerrorChild->hWnd);
				break;
			case WM_ENABLE:
				if (win->TerrorChild)
					EnableWindow(win->TerrorChild->hWnd,TRUE);
				break;
			/*case WM_ACTIVATE:
				if (win->TerrorChild)
					SetActiveWindow(win->TerrorChild->hWnd);
				break;*/
			case WM_SIZE:
				// automatically reposition statusbar and toolbar
				if (win->StatusBarEnabled)
					SendMessage(win->status_bar,(UINT)WM_SIZE,0,0);
				if (win->tool_bar[HuiToolBarTop].Enabled)
					SendMessage(win->tool_bar[HuiToolBarTop].hWnd,(UINT)TB_AUTOSIZE,0,0);
				return DefWindowProc(hwnd,message,wParam,lParam);
			default:
				return DefWindowProc(hwnd,message,wParam,lParam);
		}
		return 0;
	}

	//msg_write>Write(string2("win %d",message));

	// default cursor...
	switch (message){
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			win->mx=LOWORD(lParam);
			win->my=HIWORD(lParam);
			break;
	}

	win_proc_force_return=false;
	return 0;
}

static LRESULT WindowProcedureWithMF(HuiWindow *win,message_function *mf,HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	win_proc_force_return=true;

	
	//msg_write>Write(string2("mf   %d",message));
	switch (message){
		case WM_COMMAND:

			// control item ids
			for (int i=0;i<win->Control.size();i++){
				if (win->Control[i].hWnd==(HWND)lParam){

					// default actions
					switch (HIWORD(wParam)){
						case CBN_SELCHANGE:
						case TCN_SELCHANGE:
						case BN_CLICKED:
						case EN_CHANGE:
							if (win->ready)
								mf(win->Control[i].ID);
							return 0;
					}
					if (win->ready)
						mf(win->Control[i].ID);
    			}
				//msg_write>Write(string2("hw wP = %d",(int)HIWORD(wParam)));
			}

			// menu and toolbar ids
			if (HIWORD(wParam)==0){//WM_USER){
				if (TestMenuID(win->Menu,LOWORD(wParam),mf))
					return 0;
				if (TestMenuID(win->Popup,LOWORD(wParam),mf))
					return 0;
				if (TestToolBarID(win,LOWORD(wParam),mf))
					return 0;
			}
			break;
		case WM_NOTIFY:

			// more control item actions and ids
			for (int i=0;i<win->Control.size();i++)
				if (((LPNMHDR)lParam)->hwndFrom==win->Control[i].hWnd){

					// double click list view
					if ((win->Control[i].Kind==HuiKindListView)&&(((LPNMHDR)lParam)->code==NM_DBLCLK))
						if (win->ready)
							mf(win->Control[i].ID);

					// select tab page
					if ((win->Control[i].Kind==HuiKindTabControl)&&(((LPNMHDR)lParam)->code==TCN_SELCHANGE)){
						UpdateTabPages(win);
						if (win->ready)
							mf(win->Control[i].ID);
						return 0;
					}
				}
               DefWindowProc(hwnd,message,wParam,lParam);
		case WM_MOUSEMOVE:
			if (win->ready)
				mf(HUI_WIN_MOUSEMOVE);
			break;
		case WM_MOUSEWHEEL:
			if (win->ready)
				mf(HUI_WIN_MOUSEWHEEL);
			break;
		case WM_LBUTTONDOWN:
			if (win->ready)
				mf(HUI_WIN_LBUTTONDOWN);
			break;
		case WM_LBUTTONUP:
			if (win->ready)
				mf(HUI_WIN_LBUTTONUP);
			break;
		case WM_RBUTTONDOWN:
			if (win->ready)
				mf(HUI_WIN_RBUTTONDOWN);
			break;
		case WM_RBUTTONUP:
			if (win->ready)
				mf(HUI_WIN_RBUTTONUP);
			break;
		case WM_KEYDOWN:
			// quite complicated...
			if (GetActiveWindow()==win->hWnd){
				win->AllowKeys=true;
				// ...save to a key buffer!
				add_key_to_buffer(&win->OwnData, HuiKeyID[wParam]);
				if (win->ready)
					mf(HUI_WIN_KEYDOWN);
			}
			break;
		case WM_KEYUP:
			if (win->ready)
				mf(HUI_WIN_KEYUP);
			break;
		case WM_SIZE:
		case WM_SIZING:
			// automatically reposition statusbar and toolbar
			if (win->StatusBarEnabled)
				SendMessage(win->status_bar,(UINT)WM_SIZE,0,0);
			if (win->tool_bar[HuiToolBarTop].Enabled)
				SendMessage(win->tool_bar[HuiToolBarTop].hWnd,(UINT)TB_AUTOSIZE,0,0);
			if (win->ready)
				mf(HUI_WIN_SIZE);
               //DefWindowProc(hwnd,message,wParam,lParam);
			break;
		case WM_MOVE:
		case WM_MOVING:
			if (win->ready)
				mf(HUI_WIN_MOVE);
			break;
		case WM_PAINT:
			//if (!win->UsedByNix)
				DefWindowProc(hwnd,message,wParam,lParam); // else message boxes wouldn't work
			if (win->ready)
				mf(HUI_WIN_RENDER);
			break;
		case WM_ERASEBKGND:
			if (!win->UsedByNix)
				DefWindowProc(hwnd,message,wParam,lParam);
			//mf(HUI_WIN_ERASEBKGND);
			break;
		//case WM_DESTROY:
		//	mf(HUI_WIN_DESTROY);
		//	break;
		case WM_CLOSE:
			if (win->ready)
				mf(HUI_WIN_CLOSE);
			break;
		case WM_NCACTIVATE:
			// reactivate key handling if we get the focus
			if (win->ready)
				win->AllowKeys=true;
			return DefWindowProc(hwnd,message,wParam,lParam);
		default:
			//mf(HUI_WIN_EMPTY);
			return DefWindowProc(hwnd,message,wParam,lParam);
	}

	win_proc_force_return=false;
	return 0;
}

static LRESULT WindowProcedureNoMF(HuiWindow *win,HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	win_proc_force_return=true;
	
	//msg_write>Write(string2("!mf   %d",message));

	// default message_function (when none is applied)
	bool allow_exit=true;
	switch (message){
		case WM_NOTIFY:
			for (int i=0;i<win->Control.size();i++)
				if (((LPNMHDR)lParam)->hwndFrom==win->Control[i].hWnd){
					// select tab page
					if ((win->Control[i].Kind==HuiKindTabControl)&&(((LPNMHDR)lParam)->code==TCN_SELCHANGE)){
						UpdateTabPages(win);
						return 0;
					}
				}
			break;
		case WM_DESTROY:
			for (int i=0;i<_HuiWindow_.size();i++)
				if (_HuiWindow_[i]->MessageFunction)
					allow_exit=false;
			if (allow_exit)
				HuiEnd();
			if (win){
				win->hWnd=NULL;
				delete(win);
			}
			break;
		case WM_SIZE:
			if (win){
				// automatically reposition statusbar and toolbar
				if (win->StatusBarEnabled)
					SendMessage(win->status_bar,(UINT)WM_SIZE,0,0);
				if (win->tool_bar[HuiToolBarTop].Enabled)
					SendMessage(win->tool_bar[HuiToolBarTop].hWnd,(UINT)TB_AUTOSIZE,0,0);
			}
			return DefWindowProc(hwnd,message,wParam,lParam);
		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}

	win_proc_force_return=false;
	return 0;
}

static LRESULT CALLBACK WindowProcedure(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	//msg_write>Write("WP");

	// find hui window for hwnd...
	HuiWindow *win=NULL;
	message_function *mf=NULL;
	for (int i=0;i<_HuiWindow_.size();i++){
		if (_HuiWindow_[i]->hWnd==hwnd){
			win=_HuiWindow_[i];
			mf=win->MessageFunction;
			break;
		}
	}

	HRESULT hr;

	// default stuff
	if (win){
		hr=WindowProcedureDefaultStuff(win,hwnd,message,wParam,lParam);
		if (win_proc_force_return)
			return hr;
	}
	//msg_write>Write(" a");

	/*if (HuiIdleFunction)
		HuiIdleFunction(0);*/


	// keyboard and shortcuts handling
	if (win)
		if ((GetActiveWindow()==win->hWnd)&&(win->AllowKeys)){
			win->OwnDataOld=win->OwnData;
			for (int i=0;i<256;i++)
				win->OwnData.key[HuiKeyID[i]]=((GetAsyncKeyState(i)&(1<<15))!=0);
			int k=-1;
			if ((!win->GetKey(KEY_RALT))&&(!win->GetKey(KEY_LALT))){
				for (int i=6;i<HUI_NUM_KEYS;i++)
					if (win->GetKeyDown(i)){
						k=i;
						if ((win->GetKey(KEY_RCONTROL))||(win->GetKey(KEY_LCONTROL)))	k+=256;
						if ((win->GetKey(KEY_RSHIFT))||(win->GetKey(KEY_LSHIFT)))		k+=512;
					}
				if (k>=0)
					for (int i=0;i<HuiKeyCode.size();i++)
						if (k==HuiKeyCode[i].Code){
							//msg_write>Write("---------------------------------");
							//msg_write>Write(HuiKeyCode[i].ID);
							mf(HuiKeyCode[i].ID);
						}
			}
		}


	// with message function
	if ((mf)&&(allow_signal_level<=0)){

		hr=WindowProcedureWithMF(win,mf,hwnd,message,wParam,lParam);
		if (win_proc_force_return)
			return hr;

	// without message function -> default
	}else{

		hr=WindowProcedureNoMF(win,hwnd,message,wParam,lParam);
		if (win_proc_force_return)
			return hr;

	}
	//msg_write>Write(" x");

	/*	//if (GetActiveWindow()!=_HuiWindow_->hWnd)
		//	AllowWindowsKeyInput=false;
	
		//if (AllowWindowsKeyInput)
	//	if (GetActiveWindow()==win->hWnd)
			for (i=0;i<256;i++)
				win->OwnData.key[KeyID[i]]=(GetAsyncKeyState(i)!=0);
		else{
			for (i=0;i<256;i++)
				win->OwnData.key[i]=false;
		}*/

		// Korrektur (manche Tasten belegen mehrere Array-Elemente) :-S
		//if (GetKey(KEY_RALT))
		//	_HuiWindow_->OwnData.key[KEY_LCONTROL]=0;

    return 0;
}



//----------------------------------------------------------------------------------
// window functions


// general window
HuiWindow::HuiWindow(const char *title, int x, int y, int width, int height, HuiWindow *root, bool allow_root, int mode, bool show, message_function *mf)
{
	_Init_(root, allow_root, mf);

	
	ready = false;
	gl_hwnd = NULL;
	status_bar = NULL;
	cdx = cdy = 0;
	NixGetInputFromWindow = NULL;
	TCHAR ClassName[64];
	_tcscpy(ClassName, sys_str(string2("HuiWindowClass %d", win_reg_no ++)));
	WNDCLASSEX wincl;
	wincl.hInstance = hui_win_instance;
	wincl.lpszClassName = ClassName;
	wincl.lpfnWndProc = WindowProcedure;
	wincl.style = CS_DBLCLKS;
	wincl.cbSize = sizeof(WNDCLASSEX);

	wincl.hIcon = hui_win_main_icon;
	wincl.hIconSm = hui_win_main_icon;
	/*wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);*/

	wincl.hCursor = LoadCursor(NULL,IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	if ((mode & HuiWinModeNix) > 0)
		wincl.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	else if ((mode & HuiWinModeBGDialog) > 0)
		wincl.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		//wincl.hbrBackground = GetSysColorBrush(COLOR_BTNHIGHLIGHT);
	else // default
		wincl.hbrBackground = GetSysColorBrush(COLOR_WINDOW);

	if (!RegisterClassEx(&wincl)){
		msg_error("new HuiWindow - RegisterClassEx");
		return;
	}
		
	DWORD style_ex = 0;
	if ((mode & HuiWinModeBGDialog) > 0)
		style_ex |= WS_EX_DLGMODALFRAME | WS_EX_NOPARENTNOTIFY;
	DWORD style = WS_OVERLAPPEDWINDOW;
	if ((mode & HuiWinModeBGDialog) > 0)
		style = DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU;

	// align dialog box
	if ((mode & HuiWinModeBGDialog) > 0){
		if (root){
			// center on root window
			irect r=root->GetOuterior();
			x=r.x1+(r.x2-r.x1-width)/2;
			y=r.y1+(r.y2-r.y1-height)/2;
		}else{
			// center on screen
			DEVMODE mode;
			EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&mode);
			x=(mode.dmPelsWidth-width)/2;
			y=(mode.dmPelsHeight-height)/2;
		}
	}

	hWnd=CreateWindowEx(	style_ex,
							ClassName,
							sys_str(title),
							style,
							x<0?CW_USEDEFAULT:x,
							y<0?CW_USEDEFAULT:y,
							width<0?CW_USEDEFAULT:width,
							height<0?CW_USEDEFAULT:height,
							root?root->hWnd:HWND_DESKTOP,
							NULL,
							hui_win_instance,
							NULL);
	if (!hWnd){
		msg_error("new HuiWindow - CreateWindowEx");
		return;
	}
	ShowWindow(hWnd,SW_HIDE);


	// status bar
	status_bar=CreateWindow(STATUSCLASSNAME,_T(""),
							WS_CHILD, // | SBARS_SIZEGRIP,
							0,0,0,0,
							hWnd,NULL,hui_win_instance,NULL);

	// tool bar(s)
	tool_bar[HuiToolBarTop].hWnd=CreateWindowEx(0,TOOLBARCLASSNAME,NULL,
								//WS_CHILD | TBSTYLE_FLAT | CCS_LEFT | CCS_VERT,
								WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS,
								0, 0, 0, 0,
								hWnd,(HMENU)0,hui_win_instance,NULL);
	//RECT rr;
	//SendMessage(tool_bar[HuiToolBarTop].hWnd, TB_SETROWS, MAKEWPARAM(13, FALSE), (LPARAM)&rr);
	TBADDBITMAP bitid;
	bitid.hInst = HINST_COMMCTRL;
	bitid.nID = IDB_STD_LARGE_COLOR;//IDB_STD_SMALL_COLOR;
	SendMessage(tool_bar[HuiToolBarTop].hWnd, TB_ADDBITMAP, 1, (long)&bitid);
	SendMessage(tool_bar[HuiToolBarTop].hWnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

	//if ((show)&&(!HuiCreateHiddenWindows))
	//	ShowWindow(hWnd,SW_SHOW);



// well,.... kind of ugly....
	//if (bg_mode==HuiBGModeStyleDialog)
		//SetOuterior(irect(x,x+width+4,y,y+height+22));
}


// dummy window
HuiWindow::HuiWindow(const char *title,int x,int y,int width,int height,message_function *mf)
{
}

HuiWindow::~HuiWindow()
{
	_CleanUp_();
	
	if (Root){
		Root->Allowed=true;
		Root->TerrorChild=NULL;
		//Root->Activate();
		/*SetFocus(Root->hWnd);
		UpdateWindow(Root->hWnd);
		ShowWindow(Root->hWnd,SW_SHOW);*/
		//Root->Update();
		for (int i=0;i<Root->SubWindow.size();i++)
			if (Root->SubWindow[i]==this){
				Root->SubWindow.erase(Root->SubWindow.begin() + i);
				break;
			}
	}
	if (hWnd){
		for (int i=0;i<Control.size();i++)
			if (Control[i].hWnd)
				DestroyWindow(Control[i].hWnd);
		DestroyWindow(hWnd);
	}
}

// should be called after creating (and filling) the window to actually show it
void HuiWindow::Update()
{

	// cruel hack!!!!
	//     overrule windows behavior
	irect ro=GetOuterior();
	RECT ri;
	GetClientRect(hWnd,&ri);
	ro.x2+=(ro.x2-ro.x1)-ri.right;
	ro.y2+=(ro.y2-ro.y1)-ri.bottom;
	SetOuterior(ro);

	if (Menu)
		SetMenu(hWnd,Menu->hMenu);
	else
		SetMenu(hWnd,NULL);
	for (int i=0;i<Control.size();i++)
		if (Control[i].Kind==HuiKindTabControl)
			TabCtrl_SetCurSel(Control[i].hWnd,1);
	UpdateTabPages(this);
	for (int i=0;i<Control.size();i++)
		if (Control[i].Kind==HuiKindTabControl)
			TabCtrl_SetCurSel(Control[i].hWnd,0);
	UpdateWindow(hWnd);
	UpdateTabPages(this);
	if (IsHidden)
	    ShowWindow(hWnd,SW_HIDE);
	else
	    ShowWindow(hWnd,SW_SHOW);

	/*for (int i=0;i<NumControls;i++)
		if (Control[i].Kind==HuiKindListView){
			LVCOLUMN col;
			for (int j=0;j<128;j++)
				if (ListView_GetColumn(Control[i].hWnd,j,&col))
					ListView_SetColumnWidth(Control[i].hWnd,j,LVSCW_AUTOSIZE_USEHEADER);
				else
					break;
		}*/

	UpdateTabPages(this);
	ready = true;
	AllowInput=true;
}

// show/hide without closing the window
void HuiWindow::Hide(bool hide)
{
	if (hide)
	    ShowWindow(hWnd,SW_HIDE);
	else
	    ShowWindow(hWnd,SW_SHOW);
	IsHidden=hide;
}

// set the string in the title bar
void HuiWindow::SetTitle(const char *title)
{
	SetWindowText(hWnd,sys_str(title));
}

// identify window (for automatic title assignment with language strings)
void HuiWindow::SetID(int id)
{
	ID=id;
	if ((HuiLanguaged)&&(id>=0))
		SetTitle(HuiGetLanguage(id));
}

// set the upper left corner of the window in screen corrdinates
void HuiWindow::SetPosition(int x,int y)
{
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hWnd,&lpwndpl);
	int w=lpwndpl.rcNormalPosition.right-lpwndpl.rcNormalPosition.left;
	int h=lpwndpl.rcNormalPosition.bottom-lpwndpl.rcNormalPosition.top;
	// nicht maximiert!!
	lpwndpl.showCmd=SW_SHOW;
	lpwndpl.rcNormalPosition.left=x;
	lpwndpl.rcNormalPosition.top=y;
	lpwndpl.rcNormalPosition.right=x+w;
	lpwndpl.rcNormalPosition.bottom=y+h;
	SetWindowPlacement(hWnd,&lpwndpl);
}

// align window relative to another window (like..."top right corner")
void HuiWindow::SetPositionSpecial(HuiWindow *win,int mode)
{
	irect rp=win->GetOuterior();
	irect ro=GetOuterior();
	int x=ro.x1,y=ro.y1;
	if ((mode & HuiLeft)>0)
		x=rp.x1 + 2;
	if ((mode & HuiRight)>0)
		x=rp.x2 - (ro.x2-ro.x1) -2;
	if ((mode & HuiTop)>0)
		y=rp.y1 + 20;
	if ((mode & HuiBottom)>0)
		y=rp.y2 - (ro.y2-ro.y1) -2;
	SetPosition(x,y);
}

// set the current window position and size (including the frame and menu/toolbars...)
//    if maximized this will un-maximize the window!
void HuiWindow::SetOuterior(irect rect)
{
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hWnd,&lpwndpl);
	// not maximized!!
	lpwndpl.showCmd=SW_SHOW;
	lpwndpl.rcNormalPosition.left=rect.x1;
	lpwndpl.rcNormalPosition.top=rect.y1;
	lpwndpl.rcNormalPosition.right=rect.x2;
	lpwndpl.rcNormalPosition.bottom=rect.y2;
	SetWindowPlacement(hWnd,&lpwndpl);
}

// get the current window position and size (including the frame and menu/toolbars...)
irect HuiWindow::GetOuterior()
{
	irect r;
	RECT rect;
	GetWindowRect(hWnd,&rect);
	r.x1=rect.left;
	r.y1=rect.top;
	r.x2=rect.right;
	r.y2=rect.bottom;
	return r;
}

// set the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <SetOuterior>
void HuiWindow::SetOuteriorDesired(irect rect)
{
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hWnd,&lpwndpl);
	lpwndpl.rcNormalPosition.left=rect.x1;
	lpwndpl.rcNormalPosition.top=rect.y1;
	lpwndpl.rcNormalPosition.right=rect.x2;
	lpwndpl.rcNormalPosition.bottom=rect.y2;
	SetWindowPlacement(hWnd,&lpwndpl);
}

// get the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <GetOuterior>
irect HuiWindow::GetOuteriorDesired()
{
	irect r;
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hWnd,&lpwndpl);
	r.x1=lpwndpl.rcNormalPosition.left;
	r.y1=lpwndpl.rcNormalPosition.top;
	r.x2=lpwndpl.rcNormalPosition.right;
	r.y2=lpwndpl.rcNormalPosition.bottom;
	return r;
}

int _tool_bar_size(sHuiToolBar *tool_bar)
{
	int s=32-4;
	if (tool_bar->LargeIcons)
		s+=8;
	if (tool_bar->TextEnabled)
		s+=16;
	return s;
}

// get the "usable" part of the window: controllers/graphics area
//   relative to the outer part!!
irect HuiWindow::GetInterior()
{
	irect r;
	RECT WindowClient,ToolBarRect;
	GetClientRect(hWnd,&WindowClient);
	POINT p;
	p.x=WindowClient.left;
	p.y=WindowClient.top;
	//ClientToScreen(hWnd,&p);
	r.x1=p.x;
	r.y1=p.y;
	p.x=WindowClient.right;
	p.y=WindowClient.bottom;
	//ClientToScreen(hWnd,&p);
	r.x2=p.x;
	r.y2=p.y;
	if (tool_bar[HuiToolBarTop].Enabled){
		SendMessage(tool_bar[HuiToolBarTop].hWnd,TB_AUTOSIZE,0,0);
		GetWindowRect(tool_bar[HuiToolBarTop].hWnd,&ToolBarRect);
		int tbh=ToolBarRect.bottom-ToolBarRect.top;
		r.y1+=tbh;
	}
	return r;
}

void HuiWindow::ShowCursor(bool show)
{
	int s=::ShowCursor(show);
	if (show){
		while(s<0)
			s=::ShowCursor(show);
	}else{
		while(s>=0)
			s=::ShowCursor(show);
	}
}

// relative to Interior
void HuiWindow::SetCursorPos(int x,int y)
{
	irect ri = GetInterior();
	irect ro = GetOuterior();
	::SetCursorPos(x + ri.x1, y + ri.y1);
	
	InputData.x = (float)x;
	InputData.y = (float)y;
	InputData.dx = 0;
	InputData.dy = 0;
}

void HuiWindow::SetMaximized(bool maximized)
{
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hWnd,&lpwndpl);
	lpwndpl.showCmd=(maximized?SW_SHOWMAXIMIZED:SW_SHOW);
	SetWindowPlacement(hWnd,&lpwndpl);
}

bool HuiWindow::IsMaximized()
{
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hWnd,&lpwndpl);
	return (lpwndpl.showCmd==SW_SHOWMAXIMIZED);
}

bool HuiWindow::IsMinimized()
{
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hWnd,&lpwndpl);
	return ((lpwndpl.showCmd==SW_SHOWMINIMIZED)||(lpwndpl.showCmd==SW_MINIMIZE));
}

void HuiWindow::SetFullscreen(bool fullscreen)
{
		if (fullscreen){
			// save window data
			WindowStyle = GetWindowLong(hWnd,GWL_STYLE);
			//hMenu=GetMenu(hWnd);
			GetWindowRect(hWnd,&WindowBounds);
			GetClientRect(hWnd,&WindowClient);
			DWORD style = WS_POPUP|WS_SYSMENU|WS_VISIBLE;
			//SetWindowLong(hWnd,GWL_STYLE,WS_POPUP);
			SetWindowLong(hWnd,GWL_STYLE,style);

			WINDOWPLACEMENT wpl;
			GetWindowPlacement(hWnd,&wpl);
			wpl.rcNormalPosition.left=0;
			wpl.rcNormalPosition.top=0;
			wpl.rcNormalPosition.right=1024;//xres;
			wpl.rcNormalPosition.bottom=768;//yres;
			AdjustWindowRect(&wpl.rcNormalPosition, style, FALSE);
			SetWindowPlacement(hWnd,&wpl);
		}
}

void HuiWindow::EnableStatusBar(bool enabled)
{
	if (enabled)
		ShowWindow(status_bar,SW_SHOW);
	else
		ShowWindow(status_bar,SW_HIDE);
	StatusBarEnabled=enabled;
}

void HuiWindow::SetStatusText(const char *str)
{
	SendMessage(status_bar,SB_SETTEXT,0,(LPARAM)sys_str(str));
}

// give our window the focus....and try to focus the specified control item
void HuiWindow::Activate(int control_id)
{
	SetFocus(hWnd);
	WindowStyle=GetWindowLong(hWnd,GWL_STYLE);
	if ((WindowStyle & WS_MINIMIZE)>0)
		ShowWindow(hWnd,SW_RESTORE);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	SetForegroundWindow(hWnd);
	/*if (control_id>=0){
		for (int i=0;i<NumControls;i++)
			if (control_id==ControlID[i])
				SetFocus(Control[i]);
	}*/
}

bool HuiWindow::IsActive(bool include_sub_windows)
{
	bool ia=false;
	ia=(GetActiveWindow()==hWnd);
	if ((!ia)&&(include_sub_windows)){
		for (int i=0;i<SubWindow.size();i++)
			if (SubWindow[i]->IsActive(true))
				return true;
	}
	return ia;
}




#endif



};

#endif
