/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.07.14 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "Controls/Control.h"
#include "hui.h"
#include "internal.h"
#include "Toolbar.h"


namespace hui {


Array<Window*> _all_windows_;

Window *CurWindow = nullptr;


void InputData::reset() {
	x = y = dx = dy = scroll_x = scroll_y = 0;
	pressure = 0;
	lb = mb = rb = false;
	memset(key, 0, sizeof(key));
	key_buffer.clear();
	key_code = 0;
	just_focused = false;
}

Window::Window() {
	_init_("", 0, 0, nullptr, true, WIN_MODE_DUMMY);
}

Window::Window(const string &title, int width, int height) {
	_init_(title, width, height, nullptr, true, 0);
}

void Window::__init_ext__(const string& title, int width, int height) {
	new(this) Window(title, width, height);
}


// resource constructor
Window::Window(const string &id, Window *parent) {
	Resource *res = GetResource(id);
	if (!res) {
		msg_error("Window: undefined resource id: " + id);
		return;
	}
	_init_("", 300, 200, parent, res->has("allowparent"), 0);

	set_from_resource(res);
}

void Window::_init_generic_(Window *_root, bool _allow_root, int _mode) {
	_MakeUsable_();
	_all_windows_.add(this);

	allowed = true;
	allow_keys = true;
	parent_window = _root;
	main_input_control = nullptr;
	if (parent_window) {
		parent_window->allowed = _allow_root;
	}
	menu = popup = nullptr;
	statusbar_enabled = false;
	toolbar[TOOLBAR_TOP] = new Toolbar(this);
	toolbar[TOOLBAR_LEFT] = new Toolbar(this, true);
	toolbar[TOOLBAR_RIGHT] = new Toolbar(this, true);
	toolbar[TOOLBAR_BOTTOM] = new Toolbar(this);
	input.reset();

	allow_input = false; // allow only if ->Show() was called
}

void Window::_clean_up_() {
	for (int i=0; i<4; i++)
		delete(toolbar[i]);

	_ClearPanel_();
	input.reset();
	
	// unregister window
	for (int i=0;i<_all_windows_.num;i++)
		if (_all_windows_[i] == this) {
			_all_windows_.erase(i);
			break;
		}
}

// default handler when trying to close the windows
void Window::on_close_request() {
	destroy();
	
	// no message function (and last window): end program
	if (_all_windows_.num > 0)
		return;
	Application::hard_end();
}


// identify window (for automatic title assignment with language strings)
void Window::set_id(const string &_id) {
	id = _id;
	if (_using_language_ and (id.num > 0))
		set_title(GetLanguage(id, id));
}

// align window relative to another window (like..."top right corner")
void Window::set_position_special(Window *win,int mode) {
	int pw, ph, cw, ch, px, py, cx, cy;
	win->get_size(pw, ph);
	win->get_position(px, py);
	get_size(cw, ch);
	get_position(cx, cy);
	if ((mode & HUI_LEFT) > 0)
		cx = px + 2;
	if ((mode & HUI_RIGHT) > 0)
		cx = px + pw - cw - 2;
	if ((mode & HUI_TOP) > 0)
		cy = py + 20;
	if ((mode & HUI_BOTTOM) > 0)
		cy = py + ph - ch - 2;
	set_position(cx, cy);
}

Menu *Window::get_menu() {
	return menu;
}

Window *Window::get_parent() {
	return parent_window;
}

bool Window::get_key(int k) {
	if (k == KEY_CONTROL)
		return (input.key[KEY_RCONTROL] or input.key[KEY_LCONTROL]);
	else if (k == KEY_SHIFT)
		return (input.key[KEY_RSHIFT] or input.key[KEY_LSHIFT]);
	else
		return input.key[k];
}

bool Window::get_mouse(int &x, int &y, int button) {
	x = (int)input.x;
	y = (int)input.y;
	if (button == 0) {
		return input.lb;
	} else if (button == 1) {
		return input.mb;
	} else {
		return input.rb;
	}
}



NixWindow::NixWindow(const string& title, int width, int height) :
	Window(title, width, height)
{
	add_drawing_area("!opengl,grabfocus", 0, 0, "nix-area");
}

void NixWindow::__init_ext__(const string& title, int width, int height) {
	new(this) NixWindow(title, width, height);
}

Dialog::Dialog(const string& title, int width, int height, Window* parent, bool allow_parent) {
	_init_(title, width, height, parent, allow_parent, 0);
}

void Dialog::__init_ext__(const string& title, int width, int height, Window* parent, bool allow_parent) {
	new(this) Dialog(title, width, height, parent, allow_parent);
}



SourceWindow::SourceWindow(const string &buffer, Window *parent) {
	_init_("", 300, 200, parent, buffer.find("allow-parent"), 0);
	from_source(buffer);
}

};
