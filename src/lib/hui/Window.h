/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_WINDOW_EXISTS_
#define _HUI_WINDOW_EXISTS_

#include "Panel.h"
#include "../base/future.h"

typedef struct _GtkEventController GtkEventController;

struct vec2;
struct rect;
class Painter;

namespace hui {

class Menu;
class Event;
class Control;
class Window;
class Toolbar;
class ResourceNew;


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
class ControlBasicWindowLayout;

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
	friend class Panel;
public:
	Window();
	Window(const string &title, int width, int height);
	Window(const string &id, Window *parent);
	void _cdecl __init_ext__(const string &title, int width, int height);
	~Window() override;
	void _cdecl __delete__() override;

	void _init_(const string &title, int width, int height, Window *parent, bool allow_parent, int mode);
	void _init_generic_(Window *parent, bool allow_parent, int mode);
	void _clean_up_();
	void _wait_till_closed();

	void _cdecl request_destroy();

	// the window
	bool is_dialog();
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
	void add_basic_layout(const string& layout);
	void _cdecl set_menu(xfer<Menu> menu);
	Menu* _cdecl get_menu();
	Window* _cdecl get_parent();
	void _cdecl __set_options(const string &options);


	void _cdecl set_cursor_pos(int x,int y);
	void _cdecl show_cursor(bool show);

	// events by overwriting
	virtual void _cdecl on_mouse_move(const vec2 &m) {}
	virtual void _cdecl on_mouse_enter(const vec2 &m) {}
	virtual void _cdecl on_mouse_leave() {}
	virtual void _cdecl on_left_button_down(const vec2 &m) {}
	virtual void _cdecl on_middle_button_down(const vec2 &m) {}
	virtual void _cdecl on_right_button_down(const vec2 &m) {}
	virtual void _cdecl on_left_button_up(const vec2 &m) {}
	virtual void _cdecl on_middle_button_up(const vec2 &m) {}
	virtual void _cdecl on_right_button_up(const vec2 &m) {}
	virtual void _cdecl on_double_click(const vec2 &m) {}
	virtual void _cdecl on_mouse_wheel(const vec2 &d) {}
	virtual void _cdecl on_close_request();
	virtual void _cdecl on_key_down(int key) {}
	virtual void _cdecl on_key_up(int key) {}
	virtual void _cdecl on_draw(::Painter *p) {}

	// input
	bool _cdecl get_key(int key);
	bool _cdecl get_mouse(int &x, int &y, int button);


	// hui internal
	bool allow_input;
	InputData input;
	int mouse_offset_x, mouse_offset_y;
	Control *main_input_control;

	Toolbar *_cdecl get_toolbar(int index);

private:


public:
	GtkWidget *window;
public:
	shared<Control> header_bar;
	void _add_headerbar();
	ControlBasicWindowLayout *basic_layout = nullptr;
	
protected:
	Menu *popup;
	bool allowed, allow_keys;
	Window *parent_window;

	Array<EventKeyCode> event_key_codes;
public:
	bool requested_destroy;
	void _cdecl set_key_code(const string &id, int key_code, const string &image = "");
	void _cdecl add_action_checkable(const string &id);
	Array<EventKeyCode> get_event_key_codes() const { return event_key_codes; }
protected:

//#if GTK_CHECK_VERSION(4,0,0)
	GtkEventController *shortcut_controller = nullptr;
//#endif

public:
	base::promise<void> end_run_promise;

	void _try_send_by_key_code_(int key_code);
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


void fly_and_wait(shared<Window> win);
base::future<void> fly(shared<Window> win);



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
