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
#include "../os/msg.h"


namespace hui {


Array<Window*> _all_windows_;

Window *CurWindow = nullptr;

void DBDEL_X(const string &);


void InputData::reset() {
	x = y = dx = dy = scroll_x = scroll_y = 0;
	pressure = 0;
	lb = mb = rb = false;
	memset(key, 0, sizeof(key));
	key_buffer.clear();
	key_code = 0;
	just_focused = false;
	inside = inside_smart = false;
}

Window::Window() : Panel() {
	_init_("", 0, 0, nullptr, true, WIN_MODE_DUMMY);
}

Window::Window(const string &title, int width, int height) : Panel() {
	_init_(title, width, height, nullptr, true, 0);
}

void Window::__init_ext__(const string& title, int width, int height) {
	new(this) Window(title, width, height);
}


// resource constructor
Window::Window(const string &id, Window *parent) : Panel(id, nullptr) {
	Resource *res = get_resource(id);
	if (!res) {
		msg_error("Window: undefined resource id: " + id);
		return;
	}
	_init_("", 300, 200, parent, res->has("allowparent"), 0);

	set_from_resource(res);
}

void Window::_init_generic_(Window *_parent, bool _allow_root, int _mode) {
	_MakeUsable_();
	_all_windows_.add(this);

	allowed = true;
	allow_keys = true;
	parent_window = _parent;
	main_input_control = nullptr;
	if (parent_window) {
		parent_window->allowed = _allow_root;
	}
	popup = nullptr;
	input.reset();

	allow_input = false; // allow only if ->Show() was called
}

void Window::_clean_up_() {
}

// default handler when trying to close the windows
void Window::on_close_request() {
	request_destroy();
}


// identify window (for automatic title assignment with language strings)
void Window::set_id(const string &_id) {
	id = _id;
	if (_using_language_ and (id.num > 0))
		set_title(get_language(id, id));
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

Window *Window::get_parent() {
	return parent_window;
}

bool Window::is_dialog() {
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
	set_border_width(0);
	add_drawing_area("!opengl,mainwindowcontrol", 0, 0, "nix-area");
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


Dialog::Dialog(const string &id, Window *parent) : Window(id, parent) {}


SourceWindow::SourceWindow(const string &buffer, Window *parent) {
	_init_("", 300, 200, parent, buffer.find("allow-parent"), 0);
	from_source(buffer);
}

};
