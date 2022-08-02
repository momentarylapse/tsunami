#include "Controls/Control.h"
#include "Controls/ControlHeaderBar.h"
#include "hui.h"
#include "internal.h"
#include "Toolbar.h"
#include "../base/pointer.h"
#include "../os/time.h"
#ifdef HUI_API_GTK


#ifdef OS_WINDOWS
	#include <gdk/gdkwin32.h>
#endif
#ifdef OS_LINUX
#if !GTK_CHECK_VERSION(4,0,0)
#if HAS_LIB_XLIB
	#include <gdk/gdkx.h>
#endif
#endif
#endif

namespace hui
{


void DBDEL_START(const string &type, const string &id, void *p);
void DBDEL_X(const string &m);
void DBDEL_DONE();


#if !GTK_CHECK_VERSION(3,0,0)
GtkWidget *gtk_box_new(GtkOrientation orientation, int spacing); // -> hui_window_control_gtk.cpp
#endif


unsigned int ignore_time = 0;


//----------------------------------------------------------------------------------
// window message handling

inline Window *win_from_widget(void *widget) {
	for (Window *win: _all_windows_)
		if (win->window == widget)
			return win;
	return nullptr;
}

void Window::_try_send_by_key_code_(int key_code) {
	if (key_code <= 0)
		return;
	for (auto &k: event_key_codes)
		if (key_code == k.key_code) {
			Event e = Event(k.id, k.message);
			_send_event_(&e);
		}
}

#if GTK_CHECK_VERSION(4,0,0)
static gboolean on_gtk_window_close_request(GtkWidget *widget, gpointer user_data) {
	Window *win = (Window *)user_data;
	Event e = Event("", EventID::CLOSE);
	if (win->_send_event_(&e))
		return true;
	win->on_close_request();
	return true;
}
#else
static gboolean on_gtk_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	//msg_write("close request...");
	Window *win = (Window *)user_data;
	Event e = Event("", EventID::CLOSE);
	//msg_write("close request...2");
	if (win->_send_event_(&e))
		return true;
	//msg_write("close request...3");
	win->on_close_request();
	return true;
}

static gboolean on_gtk_window_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
	Window *win = (Window *)user_data;
	// make sure the contro/alt/shift keys are unset

	// reset all keys
	memset(&win->input.key, 0, sizeof(win->input.key));
	/*win->input.key[KEY_RSHIFT] = win->input.key[KEY_LSHIFT] = false;
	win->input.key[KEY_RCONTROL] = win->input.key[KEY_LCONTROL] = false;
	win->input.key[KEY_RALT] = win->input.key[KEY_LALT] = false;*/
	return false;
}
#endif

static void on_gtk_window_resize(GtkWidget *widget, gpointer user_data) {
	Window *win = (Window *)user_data;
	Event e = Event("", EventID::RESIZE);
	win->_send_event_(&e);
}




//----------------------------------------------------------------------------------
// window functions



string get_gtk_action_name(const string &id, Panel *scope_panel) {
	string scope;
	if (scope_panel) {
		if (scope_panel == scope_panel->win)
			scope = "win.";
		else
			scope = p2s(scope_panel) + ".";
	}
	return format("%shuiactionX%s", scope, id.hex().replace(".", ""));
	//return format("%shuiaction('%s')", with_scope ? "win." : "", id);
}


// general window

void Window::_init_(const string &title, int width, int height, Window *_parent, bool allow_parent, int mode) {
	window = nullptr;
	win = this;
	header_bar = nullptr;
	statusbar = nullptr;
	requested_destroy = false;

	if ((mode & WIN_MODE_DUMMY) > 0)
		return;

	_init_generic_(_parent, allow_parent, mode);

	// creation
	if (is_dialog()) {
		window = gtk_dialog_new();


		if (!allow_parent)
			gtk_window_set_modal(GTK_WINDOW(window), true);//false);
#ifndef OS_WINDOWS
#if !GTK_CHECK_VERSION(3,0,0)
		gtk_widget_hide(gtk_dialog_get_action_area(GTK_DIALOG(window)));
#elif !GTK_CHECK_VERSION(3,12,0)
		gtk_widget_hide(gtk_dialog_get_action_area(GTK_DIALOG(window)));
#endif
#endif

		// dialog -> center on screen or root (if given)    ->  done by gtk....later
		gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent_window->window));
		gtk_window_set_resizable(GTK_WINDOW(window), false);
	} else {
#if GTK_CHECK_VERSION(4,0,0)
		window = gtk_window_new();
		if (Application::application) {
			gtk_application_add_window(Application::application, GTK_WINDOW(window));

			//g_action_map_add_action_entries(G_ACTION_MAP(group), entries, G_N_ELEMENTS(entries), this);
			gtk_widget_insert_action_group(window, "win", G_ACTION_GROUP(action_group));

			shortcut_controller = gtk_shortcut_controller_new ();
			gtk_shortcut_controller_set_scope(GTK_SHORTCUT_CONTROLLER(shortcut_controller), GTK_SHORTCUT_SCOPE_GLOBAL);
			gtk_widget_add_controller(window, shortcut_controller);
		}
#else
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
		gtk_window_set_resizable(GTK_WINDOW(window), true);
	}

	auto parts = split_title(title);
	gtk_window_set_title(GTK_WINDOW(window), sys_str(parts[0]));

	// size
#if GTK_CHECK_VERSION(4,0,0)
	gtk_window_set_default_size(GTK_WINDOW(window), width, height);
#else
	gtk_window_resize(GTK_WINDOW(window), width, height);
	desired_width = width;
	desired_height = height;
#endif

	// icon
	string logo = Application::get_property("logo");
#if !GTK_CHECK_VERSION(4,0,0)
	if (logo.num > 0)
		gtk_window_set_icon_from_file(GTK_WINDOW(window), sys_str_f(logo), nullptr);
#endif

	// catch signals
#if GTK_CHECK_VERSION(4,0,0)
	g_signal_connect(G_OBJECT(window), "close-request", G_CALLBACK(&on_gtk_window_close_request), this);
#else
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(&on_gtk_window_close), this);
	g_signal_connect(G_OBJECT(window), "focus-in-event", G_CALLBACK(&on_gtk_window_focus), this);
#endif
	//g_signal_connect(G_OBJECT(window), "state-flags-changed", G_CALLBACK(&on_gtk_window_resize), this);

	// fill in some stuff
#if !GTK_CHECK_VERSION(4,0,0)
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
#endif
	if (is_dialog()) {
		vbox = gtk_dialog_get_content_area(GTK_DIALOG(window));
		plugable = vbox;
		target_control = nullptr;
	} else {
		vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		//gtk_box_pack_start(GTK_BOX(window), vbox, TRUE, TRUE, 0);
#if GTK_CHECK_VERSION(4,0,0)
		gtk_window_set_child(GTK_WINDOW(window), vbox);
#else
		gtk_container_add(GTK_CONTAINER(window), vbox);
#endif
		gtk_widget_show(vbox);

#if GTK_CHECK_VERSION(4,0,0)
		plugable = vbox;



		auto menu = g_menu_new();
		auto m1 = g_menu_new();
		g_menu_append(m1, "test", nullptr);
		//g_menu_append_submenu(menu, "A", G_MENU_MODEL(m1));
		menubar = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(menu));
		gtk_box_append(GTK_BOX(vbox), menubar);

		gtk_box_append(GTK_BOX(vbox), toolbar[TOOLBAR_TOP]->widget);

		plugable = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_widget_show(plugable);
		//gtk_container_set_border_width(GTK_CONTAINER(plugable), 0);
		gtk_box_append(GTK_BOX(vbox), plugable);
#else
		// menu bar
		menubar = gtk_menu_bar_new();
		gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

		// tool bars
	#if GTK_CHECK_VERSION(3,0,0)
		gtk_style_context_add_class(gtk_widget_get_style_context(toolbar[TOOLBAR_TOP]->widget), "primary-toolbar");
	#endif
		gtk_box_pack_start(GTK_BOX(vbox), toolbar[TOOLBAR_TOP]->widget, FALSE, FALSE, 0);

		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
		//gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
		gtk_widget_show(hbox);

		gtk_box_pack_start(GTK_BOX(hbox), toolbar[TOOLBAR_LEFT]->widget, FALSE, FALSE, 0);

		plugable = nullptr;
		target_control = nullptr;
		// free to use...
		//target_control = hbox;
		plugable = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_widget_show(plugable);
		//gtk_container_set_border_width(GTK_CONTAINER(plugable), 0);
		gtk_box_pack_start(GTK_BOX(hbox), plugable, TRUE, TRUE, 0);

		gtk_box_pack_start(GTK_BOX(hbox), toolbar[TOOLBAR_RIGHT]->widget, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), toolbar[TOOLBAR_BOTTOM]->widget, FALSE, FALSE, 0);

		// status bar
		statusbar = gtk_statusbar_new();
		gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);
#endif
	}
}

Window::~Window() {
	DBDEL_START("window", id, this);

	for (int i=0; i<4; i++)
		toolbar[i] = nullptr;

	header_bar = nullptr;

	//_ClearPanel_();
	input.reset();

	// unregister window
	for (int i=0; i<_all_windows_.num; i++)
		if (_all_windows_[i] == this)
			_all_windows_.erase(i);

	if (window) {
		DBDEL_X("win destroy");
#if GTK_CHECK_VERSION(4,0,0)
		gtk_window_destroy(GTK_WINDOW(window));
#else
		gtk_widget_destroy(window);
#endif
		DBDEL_X("/win destroy");
	}
	DBDEL_DONE();


	// no message function (and last window): end program
//	if (_all_windows_.num == 0)
//		Application::end();
}

void Window::__delete__() {
	this->Window::~Window();
}

void Window::request_destroy() {
	if (is_dialog()) {
		gtk_dialog_response(GTK_DIALOG(window), GTK_RESPONSE_DELETE_EVENT);
	}
	requested_destroy = true;
}

// should be called after creating (and filling) the window to actually show it
void Window::show() {
	allow_input = true;
	gtk_widget_show(window);
}


void on_gtk_window_response(GtkDialog *self, gint response_id, gpointer user_data) {
	auto win = reinterpret_cast<Window*>(user_data);
	if (win->end_run_callback)
		win->end_run_callback();
	//gtk_window_destroy(GTK_WINDOW(self));
	run_later(0.01f, [win] { delete win; });
}

namespace WindowFlightManager {
	static shared_array<Window> windows;
	void add(Window *win) {
		windows.add(win);
	}
	void remove(Window *win) {
		for (int i=0; i<windows.num; i++)
			if (windows[i] == win)
				windows.erase(i);
	}
}

void on_gtk_window_response_fly(GtkDialog *self, gint response_id, gpointer user_data) {
	auto win = reinterpret_cast<Window*>(user_data);
	if (win->end_run_callback)
		win->end_run_callback();
	run_later(0.01f, [win] { WindowFlightManager::remove(win); });
}

void Window::_fly(Callback cb) {
	show();

	WindowFlightManager::add(win);

	end_run_callback = cb;

#if GTK_CHECK_VERSION(4,0,0)
	g_signal_connect(window, "response", G_CALLBACK(on_gtk_window_response_fly), this);
	gtk_window_present(GTK_WINDOW(window));
#else
	g_signal_connect(window, "response", G_CALLBACK(on_gtk_window_response_fly), this);
#endif
}

void Window::_run(Callback cb) {
	show();
	int uid = unique_id;
	//end_run_callback = cb;

/*#if GTK_CHECK_VERSION(4,0,0)
	msg_error("TODO: hui.run() gtk4");
	g_signal_connect(window, "response", G_CALLBACK(on_gtk_window_response), this);
	gtk_window_present(GTK_WINDOW(window));
#else*/
	// hmmmm, gtk_dialog_response() seems to be ignored here...?!?
	/*if (get_parent()) {
		msg_write("...dialog");
		gtk_dialog_run(GTK_DIALOG(window));
	} else {*/
		while (!requested_destroy) {
			Application::do_single_main_loop();
			os::sleep(0.005f);
		}
//	}
//#endif
	if (cb)
		cb();
	delete this;
}

void fly(Window *win, Callback cb) {
	if (!win->is_dialog())
		msg_error("hui.fly() only allowed for Dialog!");
	win->_fly(cb);
}

void run(Window *win, Callback cb) {
	win->_run(cb);
}

void Window::set_menu(Menu *_menu) {
#if GTK_CHECK_VERSION(4,0,0)
	//action_group = g_simple_action_group_new();

	if (_menu) {
		menu = _menu;

		_connect_menu_to_panel(menu);

		gtk_popover_menu_bar_set_menu_model(GTK_POPOVER_MENU_BAR(menubar), G_MENU_MODEL(menu->gmenu));
		gtk_widget_show(menubar);
	} else {
		menu = _menu;
		gtk_popover_menu_bar_set_menu_model(GTK_POPOVER_MENU_BAR(menubar), nullptr);
		gtk_widget_hide(menubar);
	}
	// only one group allowed!


#else
	// remove old menu...
	if (menu) {
		Array<Control*> list = menu->get_all_controls();
		// move items from <menu_bar> back to <Menu>
		for (int i=0;i<gtk_menu.num;i++) {
			g_object_ref(gtk_menu[i]);
			gtk_container_remove(GTK_CONTAINER(menubar), GTK_WIDGET(gtk_menu[i]));
			gtk_menu_shell_append(GTK_MENU_SHELL(menu->widget), GTK_WIDGET(gtk_menu[i]));
			g_object_unref(gtk_menu[i]);
		}
		gtk_menu.clear();
		/*menu->set_win(NULL);
		}*/
		delete(menu);
	}

	
	// insert new menu
	menu = _menu;
	if (menu) {
		menu->set_panel(this);
		gtk_widget_show(menubar);
		gtk_num_menus = menu->items.num;
		for (int i=0;i<menu->items.num;i++) {
			// move items from <Menu> to <menu_bar>
			Control *it = menu->items[i].get();
			gtk_menu.add(it->widget);
			gtk_widget_show(gtk_menu[i]);
			g_object_ref(it->widget);
			gtk_container_remove(GTK_CONTAINER(menu->widget), gtk_menu[i]);
			gtk_menu_shell_append(GTK_MENU_SHELL(menubar), gtk_menu[i]);
			g_object_unref(it->widget);
		}

	} else {
		gtk_widget_hide(menubar);
	}
#endif
}

void Window::set_key_code(const string &id, int key_code, const string &image) {
	// make sure, each id has only 1 code
	//   (multiple ids may have the same code)
	for (auto &e: event_key_codes)
		if (e.id == id) {
			e.key_code = key_code;
			return;
		}
	event_key_codes.add(EventKeyCode(id, "", key_code));

#if GTK_CHECK_VERSION(4,0,0)
	_try_add_action_(id, false);

	int mod = (((key_code&KEY_SHIFT)>0) ? GDK_SHIFT_MASK : 0) | (((key_code&KEY_CONTROL)>0) ? GDK_CONTROL_MASK : 0) | (((key_code&KEY_ALT)>0) ? GDK_ALT_MASK : 0);
	gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(shortcut_controller),
		gtk_shortcut_new(gtk_keyval_trigger_new(HuiKeyID[key_code & 255], (GdkModifierType)mod), gtk_named_action_new(get_gtk_action_name(id, this).c_str())));
#endif
}

void Window::add_action_checkable(const string &id) {
#if GTK_CHECK_VERSION(4,0,0)
	_try_add_action_(id, true);
#endif
}

// show/hide without closing the window
void Window::hide() {
	gtk_widget_hide(window);
}

// set the string in the title bar
void Window::set_title(const string &title) {
	gtk_window_set_title(GTK_WINDOW(window),sys_str(title));
}

// set the upper left corner of the window in screen corrdinates
void Window::set_position(int x, int y) {
#if !GTK_CHECK_VERSION(4,0,0)
	gtk_window_move(GTK_WINDOW(window),x,y);
#endif
}

void Window::get_position(int &x, int &y) {
#if !GTK_CHECK_VERSION(4,0,0)
	gtk_window_get_position(GTK_WINDOW(window), &x, &y);
#endif
}

void Window::set_size(int width, int height) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_window_set_default_size(GTK_WINDOW(window), width, height);
#else
	desired_width = width;
	desired_height = height;
	/*if (is_dialog())
		gtk_widget_set_size_request(window, width, height);
	else*/
		gtk_window_resize(GTK_WINDOW(window), width, height);
#endif
}

// get the current window position and size (including the frame and menu/toolbars...)
void Window::get_size(int &width, int &height) {
#if GTK_CHECK_VERSION(4,0,0)
	//gtk_widget_get_size(GTK_WIDGET(window), &width, &height);
#else
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
#endif
}

// set the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <SetOuterior>
void Window::set_size_desired(int width, int height) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_window_set_default_size(GTK_WINDOW(window), width, height);
#else
	// bad hack
	bool maximized = is_maximized();
	if (maximized)
		gtk_window_unmaximize(GTK_WINDOW(window));
	gtk_window_resize(GTK_WINDOW(window), width, height);
	if (maximized)
		gtk_window_maximize(GTK_WINDOW(window));
	desired_width = width;
	desired_height = height;
#endif
}

// get the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <GetOuterior>
void Window::get_size_desired(int &width, int &height) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_window_get_default_size(GTK_WINDOW(window), &width, &height);
#else
	if (is_maximized()) {
		width = desired_width;
		height = desired_height;
	} else {
		gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	}
#endif
}

void Window::show_cursor(bool show) {
#ifdef OS_WINDOWS
	int s=::ShowCursor(show);
	if (show) {
		while(s<0)
			s=::ShowCursor(show);
	} else {
		while(s>=0)
			s=::ShowCursor(show);
	}
#else
#if !GTK_CHECK_VERSION(4,0,0)
	if (show)
		gdk_window_set_cursor(gtk_widget_get_window(vbox), nullptr);
	else
		gdk_window_set_cursor(gtk_widget_get_window(vbox), (GdkCursor*)invisible_cursor);
#endif
#endif
}


// relative to Interior
void Window::set_cursor_pos(int x, int y) {
#if 0
	if (main_input_control) {
		//msg_write(format("set cursor %d %d  ->  %d %d", (int)input.x, (int)input.y, x, y));
		GtkAreaMouseSet = 2;
		GtkAreaMouseSetX = x;
		GtkAreaMouseSetY = y;
		input.x = (float)x;
		input.y = (float)y;
		// TODO GTK3
#if HAS_LIB_XLIB
		XWarpPointer(x_display, None, GDK_WINDOW_XID(gtk_widget_get_window(main_input_control->widget)), 0, 0, 0, 0, x, y);
		XFlush(x_display);
#endif
#ifdef OS_WINDOWS
		RECT r;
		GetWindowRect((HWND)GDK_WINDOW_HWND(gtk_widget_get_window(main_input_control->widget)), &r);
		::SetCursorPos(x + r.left, y + r.top);
#endif
	}
#endif
}

void Window::set_maximized(bool maximized) {
	if (maximized) {
#if !GTK_CHECK_VERSION(4,0,0)
		if (!is_maximized())
			gtk_window_get_size(GTK_WINDOW(window), &desired_width, &desired_height);
#endif
		gtk_window_maximize(GTK_WINDOW(window));
	} else {
		gtk_window_unmaximize(GTK_WINDOW(window));
	}
}

bool Window::is_maximized() {
	return gtk_window_is_maximized(GTK_WINDOW(window));
}

bool Window::is_minimized() {
#if GTK_CHECK_VERSION(4,0,0)
	return false;
#else
	int state = gdk_window_get_state(gtk_widget_get_window(window));
	return ((state & GDK_WINDOW_STATE_ICONIFIED) > 0);
#endif
}

void Window::set_fullscreen(bool fullscreen) {
	if (fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(window));
}

void Window::enable_statusbar(bool enabled) {
	if (enabled)
	    gtk_widget_show(statusbar);
	else
	    gtk_widget_hide(statusbar);
	statusbar_enabled = enabled;
}

void Window::set_status_text(const string &str) {
	gtk_statusbar_push(GTK_STATUSBAR(statusbar),0,sys_str(str));
}

static Array<string> __info_bar_responses;
static int make_info_bar_response(const string &id) {
	foreachi (string &_id, __info_bar_responses, i)
		if (_id == id)
			return i + 1234;
	__info_bar_responses.add(id);
	return __info_bar_responses.num - 1 + 1234;
}

void __GtkOnInfoBarResponse(GtkWidget *widget, int response, gpointer data) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_unparent(widget);
#else
	gtk_widget_destroy(widget);
#endif
	Window *win = (Window*)data;

	int index = response - 1234;
	if (index >= 0 and index < __info_bar_responses.num) {
		Event e = Event(__info_bar_responses[index], EventID::INFO);
		win->_send_event_(&e);
	}
	//win->infobar = nullptr;
	//win->
}

Window::InfoBar *Window::_get_info_bar(const string &id) {
	for (auto &i: info_bars)
		if (i.id == id) {
#if GTK_CHECK_VERSION(4,0,0)
			gtk_widget_unparent(i.widget);
#else
			gtk_widget_destroy(i.widget);
#endif
			return &i;
		}


	InfoBar i;
	i.id = id;
	info_bars.add(i);
	return &info_bars.back();
}

void Window::set_info_text(const string &str, const Array<string> &options) {
//#if !GTK_CHECK_VERSION(4,0,0)
	string id = "default";
	for (string &o: options)
		if (o.head(3) == "id=")
			id = o.sub(3);

	auto infobar = _get_info_bar(id);

	infobar->widget = gtk_info_bar_new();
#if GTK_CHECK_VERSION(4,0,0)
	gtk_box_insert_child_after(GTK_BOX(vbox), infobar->widget, menubar);
#else
	gtk_box_pack_start(GTK_BOX(vbox), infobar->widget, FALSE, FALSE, 0);
	gtk_box_reorder_child(GTK_BOX(vbox), infobar->widget, 2);
#endif
	//gtk_widget_set_no_show_all(infobar, TRUE);

	infobar->label = gtk_label_new("");
	//gtk_label_set_text(GTK_LABEL (message_label), sys_str(str));
#if GTK_CHECK_VERSION(4,0,0)
	gtk_info_bar_add_child(GTK_INFO_BAR(infobar->widget), infobar->label);
#else
	auto *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR(infobar->widget));
	gtk_container_add(GTK_CONTAINER(content_area), infobar->label);
#endif

	g_signal_connect(infobar->widget, "response", G_CALLBACK(&__GtkOnInfoBarResponse), this);

	gtk_label_set_text(GTK_LABEL (infobar->label), sys_str(str));



	GtkMessageType type = GTK_MESSAGE_INFO;
	bool allow_close = false;
	int num_buttons = 0;
	for (auto &o: options) {
		if (o == "error")
			type = GTK_MESSAGE_ERROR;
		if (o == "warning")
			type = GTK_MESSAGE_WARNING;
		if (o == "question")
			type = GTK_MESSAGE_QUESTION;
		if (o == "allow-close")
			allow_close = true;
		if (o.head(7) == "button:") {
			auto x = o.explode(":");
			if (x.num >= 3)
				gtk_info_bar_add_button(GTK_INFO_BAR(infobar->widget), sys_str(x[2]), make_info_bar_response(x[1]));
		}
	}
	gtk_info_bar_set_message_type(GTK_INFO_BAR(infobar->widget), type);
	gtk_info_bar_set_show_close_button(GTK_INFO_BAR(infobar->widget), allow_close);

	if (sa_contains(options, "clear")) {
		gtk_widget_hide(infobar->widget);
	} else {
		gtk_widget_show(infobar->widget);
		gtk_widget_show(infobar->label);
	}
//#endif
}

void Window::__set_options(const string &options) {
	auto r = parse_options(options);
	for (auto x: r) {
		auto op = x.first;
		auto val = x.second;

		if (op == "resizable") {
			gtk_window_set_resizable(GTK_WINDOW(window), val_is_positive(val, true));
		} else if (op == "closable") {
			//gtk_window_set_resizable(GTK_WINDOW(window), val_is_positive(val, true));
		} else if (op == "headerbar") {
			_add_headerbar();
		} else if (op == "statusbar") {
			enable_statusbar(val_is_positive(val, true));
		} else if (op == "closebutton") {
			if (header_bar)
				header_bar->set_options(op + "=" + val);
		} else if (op == "cursor") {
			show_cursor(val_is_positive(val, true));
		} else if (op == "borderwidth") {
			set_border_width(val._int());
		} else if (op == "spacing") {
			spacing = val._int();
		}
	}
}

void Window::_add_headerbar() {
	if (header_bar)
		return;
	header_bar = new ControlHeaderBar("", ":header:", this);
	gtk_window_set_titlebar(GTK_WINDOW(window), header_bar->widget);
}


// give our window the focus....and try to focus the specified control item
void Panel::activate(const string &control_id) {
	if (win) {
		gtk_widget_grab_focus(win->window);
		gtk_window_present(GTK_WINDOW(win->window));
	}
	if (control_id.num > 0)
		apply_foreach(control_id, [](Control *c) { c->focus(); });
}

bool Panel::is_active(const string &control_id) {
	if (control_id.num > 0) {
		bool r = false;
		apply_foreach(control_id, [&r](Control *c) { r = c->has_focus(); });
		return r;
	}
	if (!win)
		return false;
	return (bool)gtk_widget_has_focus(win->window);
}

};


#endif
