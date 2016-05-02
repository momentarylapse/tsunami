/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_WINDOW_EXISTS_
#define _HUI_WINDOW_EXISTS_

#include "hui_input.h"
#include "HuiPanel.h"

class HuiMenu;
class HuiEvent;
class HuiControl;
class HuiWindow;
class Painter;
class HuiToolbar;
class rect;
class HuiResourceNew;


struct HuiCompleteWindowMessage
{
	#ifdef HUI_API_WIN
		unsigned int msg,wparam,lparam;
	#endif
};


// user input
struct HuiInputData
{
	// mouse
	float x, y, dx, dy, dz;	// position, change
	bool inside, inside_smart;
	bool lb,mb,rb;				// buttons
	int row, column;
	// keyboard
	bool key[256];
	int key_code;
	int KeyBufferDepth;
	int KeyBuffer[HUI_MAX_KEYBUFFER_DEPTH];
	void reset()
	{	memset(this, 0, sizeof(HuiInputData));	}
};

class HuiToolbar;
class HuiControl;
class HuiControlTabControl;
class HuiControlListView;
class HuiControlTreeView;
class HuiControlGrid;
class HuiControlRadioButton;
class HuiControlGroup;
class HuiControlExpander;

class HuiWindow : public HuiPanel
{
	friend class HuiToolbar;
	friend class HuiControl;
	friend class HuiControlTabControl;
	friend class HuiControlListView;
	friend class HuiControlTreeView;
	friend class HuiControlGrid;
	friend class HuiControlRadioButton;
	friend class HuiControlGroup;
	friend class HuiControlExpander;
	friend class HuiMenu;
public:
	HuiWindow();
	HuiWindow(const string &title, int x, int y, int width, int height, HuiWindow *parent, bool allow_parent, int mode);
	HuiWindow(const string &title, int x, int y, int width, int height);
	HuiWindow(const string &id, HuiWindow *parent);
	void _cdecl __init_ext__(const string &title, int x, int y, int width, int height);
	virtual ~HuiWindow();
	virtual void _cdecl __delete__();

	void _init_(const string &title, int x, int y, int width, int height, HuiWindow *parent, bool allow_parent, int mode);
	void _init_generic_(HuiWindow *parent, bool allow_parent, int mode);
	void _clean_up_();

	// the window
	string _cdecl run();
	void _cdecl show();
	void _cdecl hide();
	void _cdecl setMaximized(bool maximized);
	bool _cdecl isMaximized();
	bool _cdecl isMinimized();
	void _cdecl setID(const string &id);
	void _cdecl setFullscreen(bool fullscreen);
	void _cdecl setTitle(const string &title);
	void _cdecl setPosition(int x, int y);
	void _cdecl setPositionSpecial(HuiWindow *win, int mode);
	void _cdecl getPosition(int &x, int &y);
	void _cdecl setSize(int width, int height);
	void _cdecl getSize(int &width, int &height);
	void _cdecl setSizeDesired(int width, int height);
	void _cdecl getSizeDesired(int &width, int &height);
	void _cdecl setMenu(HuiMenu *menu);
	HuiMenu* _cdecl getMenu();
	HuiWindow* _cdecl getParent();


	void _cdecl setCursorPos(int x,int y);
	void _cdecl showCursor(bool show);

	// status bar
	void _cdecl enableStatusbar(bool enabled);
	//bool _cdecl isStatusbarEnabled();
	void _cdecl setStatusText(const string &str);

	// events by overwriting
	virtual void _cdecl onMouseMove(){}
	virtual void _cdecl onMouseEnter(){}
	virtual void _cdecl onMouseLeave(){}
	virtual void _cdecl onLeftButtonDown(){}
	virtual void _cdecl onMiddleButtonDown(){}
	virtual void _cdecl onRightButtonDown(){}
	virtual void _cdecl onLeftButtonUp(){}
	virtual void _cdecl onMiddleButtonUp(){}
	virtual void _cdecl onRightButtonUp(){}
	virtual void _cdecl onDoubleClick(){}
	virtual void _cdecl onMouseWheel(){}
	virtual void _cdecl onCloseRequest();
	virtual void _cdecl onKeyDown(){}
	virtual void _cdecl onKeyUp(){}
	virtual void _cdecl onDraw(Painter *p){}

	// input
	bool _cdecl getKey(int key);
	bool _cdecl getMouse(int &x, int &y, int button);


	// hui internal
	int _get_main_level_();
	bool allow_input;
	HuiInputData input;
	int mouse_offset_x, mouse_offset_y;
	HuiControl *main_input_control;

	HuiToolbar *toolbar[4];

private:


#ifdef OS_WINDOWS
public:
	HWND hWnd;
private:
#endif
#ifdef HUI_API_WIN
	bool ready;
	//hui_callback *NixGetInputFromWindow;
	HWND statusbar, gl_hwnd;
	RECT WindowBounds,WindowClient;
	DWORD WindowStyle;
	int cdx,cdy;
#endif
#ifdef HUI_API_GTK
public:
	GtkWidget *window;
private:
	GtkWidget *vbox, *hbox, *menubar, *statusbar, *__ttt__;
	Array<GtkWidget*> gtk_menu;
	int gtk_num_menus;
	int desired_width, desired_height;
#endif
	
	HuiMenu *menu, *popup;
	bool statusbar_enabled;
	bool allowed, allow_keys;
	HuiWindow *parent;

	int main_level;
};


class HuiNixWindow : public HuiWindow
{
public:
	HuiNixWindow(const string &title, int x, int y, int width, int height);
	void _cdecl __init_ext__(const string &title, int x, int y, int width, int height);
};

class HuiDialog : public HuiWindow
{
public:
	HuiDialog(const string &title, int width, int height, HuiWindow *root, bool allow_root);
	void _cdecl __init_ext__(const string &title, int width, int height, HuiWindow *root, bool allow_root);
};

class HuiFixedDialog : public HuiWindow
{
public:
	HuiFixedDialog(const string &title, int width, int height, HuiWindow *root, bool allow_root);
	void _cdecl __init_ext__(const string &title, int width, int height, HuiWindow *root, bool allow_root);
};

extern HuiWindow *HuiCurWindow;


class HuiSourceDialog : public HuiWindow
{
public:
	HuiSourceDialog(const string &source, HuiWindow *root);
	void _cdecl __init_ext__(const string &source, HuiWindow *root);
};


void HuiFuncIgnore();
void HuiFuncClose();

enum{
	HuiWinModeResizable = 1,
	HuiWinModeNoFrame = 2,
	HuiWinModeNoTitle = 4,
	HuiWinModeControls = 8,
	HuiWinModeDummy = 16,
};

#define HuiLeft		1
#define HuiRight	2
#define HuiTop		4
#define HuiBottom	8



// which one of the toolbars?
enum{
	HuiToolbarTop,
	HuiToolbarBottom,
	HuiToolbarLeft,
	HuiToolbarRight
};


#endif
