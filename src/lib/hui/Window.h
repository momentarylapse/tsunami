/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_WINDOW_EXISTS_
#define _HUI_WINDOW_EXISTS_

#include "Event.h"
#include "Panel.h"


class rect;
class Painter;

namespace hui
{

class Menu;
class Event;
class Control;
class Window;
class Toolbar;
class ResourceNew;


struct CompleteWindowMessage
{
	#ifdef HUI_API_WIN
		unsigned int msg,wparam,lparam;
	#endif
};


// user input
struct InputData
{
	// mouse
	float x, y, dx, dy, scroll_x, scroll_y;	// position, change
	bool inside, inside_smart;
	bool lb, mb, rb; // buttons
	int row, column, row_target;
	// keyboard
	bool key[256];
	int key_code;
	Array<int> key_buffer;
	void reset();
};

class Toolbar;
class Control;
class ControlTabControl;
class ControlListView;
class ControlTreeView;
class ControlGrid;
class ControlRadioButton;
class ControlGroup;
class ControlExpander;

class Window : public Panel
{
	friend class Toolbar;
	friend class Control;
	friend class ControlTabControl;
	friend class ControlListView;
	friend class ControlTreeView;
	friend class ControlGrid;
	friend class ControlRadioButton;
	friend class ControlGroup;
	friend class ControlExpander;
	friend class Menu;
public:
	Window();
	Window(const string &title, int x, int y, int width, int height, Window *parent, bool allow_parent, int mode);
	Window(const string &title, int x, int y, int width, int height);
	Window(const string &id, Window *parent);
	void _cdecl __init_ext__(const string &title, int x, int y, int width, int height);
	virtual ~Window();
	virtual void _cdecl __delete__();

	void _init_(const string &title, int x, int y, int width, int height, Window *parent, bool allow_parent, int mode);
	void _init_generic_(Window *parent, bool allow_parent, int mode);
	void _clean_up_();

	void _cdecl destroy();
	bool _cdecl gotDestroyed();
	virtual void _cdecl onDestroy(){}

	// the window
	void _cdecl run();
	void _cdecl show();
	void _cdecl hide();
	void _cdecl setMaximized(bool maximized);
	bool _cdecl isMaximized();
	bool _cdecl isMinimized();
	void _cdecl setID(const string &id);
	void _cdecl setFullscreen(bool fullscreen);
	void _cdecl setTitle(const string &title);
	void _cdecl setPosition(int x, int y);
	void _cdecl setPositionSpecial(Window *win, int mode);
	void _cdecl getPosition(int &x, int &y);
	void _cdecl setSize(int width, int height);
	void _cdecl getSize(int &width, int &height);
	void _cdecl setSizeDesired(int width, int height);
	void _cdecl getSizeDesired(int &width, int &height);
	void _cdecl setMenu(Menu *menu);
	Menu* _cdecl getMenu();
	Window* _cdecl getParent();


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
	virtual void _cdecl onDraw(::Painter *p){}

	// input
	bool _cdecl getKey(int key);
	bool _cdecl getMouse(int &x, int &y, int button);


	// hui internal
	bool allow_input;
	InputData input;
	int mouse_offset_x, mouse_offset_y;
	Control *main_input_control;

	Toolbar *toolbar[4];

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
	
	Menu *menu, *popup;
	bool statusbar_enabled;
	bool allowed, allow_keys;
	Window *parent;
};


class NixWindow : public Window
{
public:
	NixWindow(const string &title, int x, int y, int width, int height);
	void _cdecl __init_ext__(const string &title, int x, int y, int width, int height);
};

class Dialog : public Window
{
public:
	Dialog(const string &title, int width, int height, Window *root, bool allow_root);
	void _cdecl __init_ext__(const string &title, int width, int height, Window *root, bool allow_root);
};

class FixedDialog : public Window
{
public:
	FixedDialog(const string &title, int width, int height, Window *root, bool allow_root);
	void _cdecl __init_ext__(const string &title, int width, int height, Window *root, bool allow_root);
};

extern Window *CurWindow;


class SourceDialog : public Window
{
public:
	SourceDialog(const string &source, Window *root);
	void _cdecl __init_ext__(const string &source, Window *root);
};


void FuncIgnore();
void FuncClose();

enum{
	WIN_MODE_RESIZABLE = 1,
	WIN_MODE_NO_FRAME = 2,
	WIN_MODE_NO_TITLE = 4,
	WIN_MODE_CONTROLS = 8,
	WIN_MODE_DUMMY = 16,
};

enum{
	HUI_LEFT = 1,
	HUI_RIGHT = 2,
	HUI_TOP = 4,
	HUI_BOTTOM = 8
};



// which one of the toolbars?
enum{
	TOOLBAR_TOP,
	TOOLBAR_BOTTOM,
	TOOLBAR_LEFT,
	TOOLBAR_RIGHT
};

};

#endif
