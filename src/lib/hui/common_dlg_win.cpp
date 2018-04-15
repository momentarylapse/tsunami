#include "hui.h"
#ifdef HUI_API_WIN

#include <ShlObj.h>
#include <tchar.h>

namespace hui
{

#define TCHAR_STRING_LENGTH			1024


//#define __T(x)  L ## x
static int CALLBACK FileDialogDirCallBack(HWND hWnd, UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	if (uMsg==BFFM_INITIALIZED){
		SendMessage(hWnd,BFFM_SETSELECTION,TRUE,lpData);
		HWND tree=FindWindowEx(hWnd,NULL,_T("SysTreeView32"),NULL);
		if (tree){
			HuiInfoBox(NULL,"","");
			RECT r;
			GetWindowRect(tree,&r);
			ScreenToClient(hWnd,(LPPOINT)&r);
			ScreenToClient(hWnd,((LPPOINT)&r)+1);
			r.top-=5;
			r.left-=5;
			r.right+=5;
			r.bottom+=5;
			MoveWindow(tree,r.left,r.top,(r.left-r.right),(r.bottom-r.top),FALSE);
		}
	}
	return 0;
}

static TCHAR _filename_[512],_complete_name_[512],_path_[512];

// Dialog zur Wahl eines Verzeichnisses (<dir> ist das anfangs ausgewaehlte)
bool HuiFileDialogDir(HuiWindow *win, const string &title, const string &dir/*, const string &root_dir*/)
{
	BROWSEINFO bi;
	ZeroMemory(&bi,sizeof(bi));
	bi.hwndOwner=win?win->hWnd:NULL;
	/*if (root_dir)
		bi.pidlRoot=*lpItemIdList;
	else*/
		bi.pidlRoot=NULL;
	bi.lpszTitle=sys_str(title);
#if _MSC_VER > 1000
	bi.ulFlags= BIF_EDITBOX | 64 | BIF_RETURNONLYFSDIRS;
#else
	bi.ulFlags=BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
#endif
	bi.lpfn=FileDialogDirCallBack;
	_tcscpy(_path_,sys_str_f(dir));
	bi.lParam=(LPARAM)_path_;//sys_str_f(dir);
	LPITEMIDLIST pidl=SHBrowseForFolder(&bi);
	if (pidl){
		SHGetPathFromIDList(pidl,_path_);//sys_str_f(FileDialogPath));
		IMalloc *imalloc=0;
		if (SUCCEEDED(SHGetMalloc(&imalloc))){
			imalloc->Free(pidl);
			imalloc->Release();
		}
	}
	HuiFilename = de_sys_str_f(_path_);

	return HuiFilename.num > 0;
}

// Datei-Auswahl zum Oeffnen (filter in der Form "*.txt")
bool HuiFileDialogOpen(HuiWindow *win, const string &title, const string &dir, const string &show_filter, const string &filter)
{
	HWND hWnd=win?win->hWnd:NULL;
	TCHAR _filter_[256];
	// filter = $show_filter\0$filter\0\0
	ZeroMemory(_filter_,sizeof(_filter_));
	_tcscpy(_filter_,sys_str(show_filter));
	_tcscpy(&_filter_[_tcslen(_filter_)+1],sys_str(filter));
	_tcscpy(_path_,sys_str_f(dir));
	_tcscpy(_filename_,_T(""));
	_tcscpy(_complete_name_,_T(""));
	OPENFILENAME ofn={	sizeof(OPENFILENAME),
						hWnd,NULL,
						_filter_,
						NULL,0,1,_complete_name_,TCHAR_STRING_LENGTH,
						_filename_,TCHAR_STRING_LENGTH,_path_,sys_str(title),OFN_FILEMUSTEXIST,0,1,_T("????"),0,NULL,NULL};
	bool done=(GetOpenFileName(&ofn)==TRUE);
	HuiFilename = de_sys_str_f(_complete_name_);
	return done;
}

static void try_to_ensure_extension(string &filename, const string &filter)
{
	// multiple choices -> ignore
	if (filter.find(";") >= 0)
		return;
	string ext = filter.substr(1, -1);
	if (filename.tail(ext.num) == ext)
		filename += ext;
}

// Datei-Auswahl zum Speichern
bool HuiFileDialogSave(HuiWindow *win, const string &title, const string &dir, const string &show_filter, const string &filter)
{
	HWND hWnd = win ? win->hWnd : NULL;
	TCHAR _filter_[256];
	// filter = $show_filter\0$filter\0\0
	ZeroMemory(_filter_,sizeof(_filter_));
	_tcscpy(_filter_,sys_str(show_filter));
	_tcscpy(&_filter_[_tcslen(_filter_)+1],sys_str(filter));
	_tcscpy( _path_,sys_str_f( dir ) );
	_tcscpy( _filename_ , _T( "" ) );
	_tcscpy( _complete_name_ , _T( "" ) );
	OPENFILENAME ofn={	sizeof( OPENFILENAME ),
						hWnd, NULL,
						_filter_,
						NULL, 0, 1, _complete_name_, TCHAR_STRING_LENGTH,
						_filename_, TCHAR_STRING_LENGTH, _path_, sys_str(title),
						OFN_FILEMUSTEXIST, 0, 1, _T( "????" ), 0, NULL, NULL };
	if ( GetSaveFileName( &ofn ) == TRUE ){
		HuiFilename = de_sys_str_f(_complete_name_);
		return true;
	}
	return false;
}

bool HuiSelectColor(HuiWindow *win, int r, int g, int b)
{
	HWND hWnd = win ? win->hWnd : NULL;
	CHOOSECOLOR cc;
	static COLORREF cust_color[16];
	ZeroMemory( &cc, sizeof( cc ) );
	cc.lStructSize = sizeof( cc );
	cc.hwndOwner = hWnd;
	cc.lpCustColors = cust_color;
	cc.rgbResult = RGB( r, g, b );
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	if ( ChooseColor( &cc ) == TRUE ){
		HuiColor[0] = ( cc.rgbResult & 0xff ); // red
		HuiColor[1] = ( ( cc.rgbResult >> 8 ) & 0xff ); // green
		HuiColor[2] = ( ( cc.rgbResult >> 16 ) & 0xff ); // blue
		HuiColor[3] = 255; // alpha.... :---(
		return true;
	}
	return false;
}

string HuiQuestionBox(HuiWindow *win, const string &title, const string &text, bool allow_cancel)
{
	HWND hWnd = win ? win->hWnd : NULL;
	int r = MessageBox(	hWnd,
						sys_str( text ),
						sys_str( title ),
						( allow_cancel ? MB_YESNOCANCEL : MB_YESNO ) | MB_ICONQUESTION );
	if (r == IDYES)
		return "hui:yes";
	if (r == IDNO)
		return "hui:no";
	return "hui:cancel";
}

void HuiInfoBox(HuiWindow *win, const string &title, const string &text)
{
	HWND hWnd = win ? win->hWnd : NULL;
	MessageBox(	hWnd,
				sys_str( text ),
				sys_str( title ),
				MB_OK | MB_ICONINFORMATION );// | MB_RTLREADING);
}

void HuiErrorBox(HuiWindow *win,const string &title,const string &text)
{
	HWND hWnd = win ? win->hWnd : NULL;
	MessageBox(	hWnd,
				sys_str( text ),
				sys_str( title ),
				MB_OK | MB_ICONERROR);
}

void HuiAboutBox(HuiWindow *win)
{
	msg_todo("HuiAboutBox (Win)");
	HuiInfoBox(win,_("Description"), HuiGetProperty("name") + " " + HuiGetProperty("version") + " " + HuiGetProperty("copyright"));
}

};

#endif
