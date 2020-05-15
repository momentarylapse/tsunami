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

namespace hui {

class Menu;
class Event;
class Control;
class Window;
class Toolbar;
class ResourceNew;


struct CompleteWindowMessage {
	#ifdef HUI_API_WIN
		unsigned int msg,wparam,lparam;
	#endif
};


// user input
struct InputData {
	// mouse
	float x, y, dx, dy, scroll_x, scroll_y;	// position, change
	float pressure;
	bool inside, inside_smart;
	bool lb, mb, rb; // buttons
	int row, column, row_target;
	bool just_focused;
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

class Window : public Panel {
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
	Window(const string &title, int width, int height);
	Window(const string &id, Window *parent);
	void _cdecl __init_ext__(const string &title, int width, int height);
	virtual ~Window();
	void _cdecl __delete__() override;

	void _init_(const string &title, int width, int height, Window *parent, bool allow_parent, int mode);
	void _init_generic_(Window *parent, bool allow_parent, int mode);
	void _clean_up_();

	void _cdecl destroy();
	bool _cdecl got_destroyed();
	virtual void _cdecl on_destroy(){}

	// the window
	void _cdecl run();
	void _cdecl show();
	void _cdecl hide();
	void _cdecl set_maximized(bool maximized);
	bool _cdecl is_maximized();
	bool _cdecl is_minimized();
	void _cdecl set_id(const string &id);
	void _cdecl set_fullscreen(bool fullscreen);
	void _cdecl set_title(const string &title);
	void _cdecl set_position(int x, int y);
	void _cdecl set_position_special(Window *win, int mode);
	void _cdecl get_position(int &x, int &y);
	void _cdecl set_size(int width, int height);
	void _cdecl get_size(int &width, int &height);
	void _cdecl set_size_desired(int width, int height);
	void _cdecl get_size_desired(int &width, int &height);
	void _cdecl set_menu(Menu *menu);
	Menu* _cdecl get_menu();
	Window* _cdecl get_parent();
	void _cdecl __set_options(const string &options);


	void _cdecl set_cursor_pos(int x,int y);
	void _cdecl show_cursor(bool show);

	// status bar
	void _cdecl enable_statusbar(bool enabled);
	//bool _cdecl is_statusbar_enabled();
	void _cdecl set_status_text(const string &str);
	void _cdecl set_info_text(const string &str, const Array<string> &options);

	// events by overwriting
	virtual void _cdecl on_mouse_move(){}
	virtual void _cdecl on_mouse_enter(){}
	virtual void _cdecl on_mouse_leave(){}
	virtual void _cdecl on_left_button_down(){}
	virtual void _cdecl on_middle_button_down(){}
	virtual void _cdecl on_right_button_down(){}
	virtual void _cdecl on_left_button_up(){}
	virtual void _cdecl on_middle_button_up(){}
	virtual void _cdecl on_right_button_up(){}
	virtual void _cdecl on_double_click(){}
	virtual void _cdecl on_mouse_wheel(){}
	virtual void _cdecl on_close_request();
	virtual void _cdecl on_key_down(){}
	virtual void _cdecl on_key_up(){}
	virtual void _cdecl on_draw(::Painter *p){}

	// input
	bool _cdecl get_key(int key);
	bool _cdecl get_mouse(int &x, int &y, int button);


	// hui internal
	bool allow_input;
	InputData input;
	int mouse_offset_x, mouse_offset_y;
	Control *main_input_control;

	Toolbar *toolbar[4];
	Toolbar *_cdecl get_toolbar(int index){ return toolbar[index]; }

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
	GtkWidget *vbox, *hbox, *menubar, *statusbar, *headerbar;
	Array<GtkWidget*> gtk_menu;
	int gtk_num_menus;
	struct InfoBar {
		GtkWidget *widget;
		GtkWidget *label;
		string id;
	};
	Array<InfoBar> info_bars;
	InfoBar *_get_info_bar(const string &id);
	void _add_headerbar();
#endif
	
protected:
	Menu *menu, *popup;
	bool statusbar_enabled;
	bool allowed, allow_keys;
	Window *parent_window;
};


class NixWindow : public Window {
public:
	NixWindow(const string &title, int width, int height);
	void _cdecl __init_ext__(const string &title, int width, int height);
};

class Dialog : public Window {
public:
	Dialog(const string &title, int width, int height, Window *parent, bool allow_parent);
	Dialog(const string &id, Window *parent);
	void _cdecl __init_ext__(const string &title, int width, int height, Window *parent, bool allow_parent);
};

extern Window *CurWindow;


class SourceWindow : public Window {
public:
	SourceWindow(const string &source, Window *parent);
	void _cdecl __init_ext__(const string &source, Window *parent);
};



enum {
	WIN_MODE_DUMMY = 16,
};

enum {
	HUI_LEFT = 1,
	HUI_RIGHT = 2,
	HUI_TOP = 4,
	HUI_BOTTOM = 8
};



// which one of the toolbars?
enum {
	TOOLBAR_TOP,
	TOOLBAR_BOTTOM,
	TOOLBAR_LEFT,
	TOOLBAR_RIGHT
};

};

#endif
