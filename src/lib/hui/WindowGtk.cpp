#include "Controls/Control.h"
#include "Controls/ControlHeaderBar.h"
#include "hui.h"
#include "internal.h"
#include "Toolbar.h"
#include "../base/pointer.h"
#include "../base/iter.h"
#include "../os/time.h"
#include "../os/msg.h"
#ifdef HUI_API_GTK

#include <gtk/gtk.h>

#ifdef OS_WINDOWS
	#include <gdk/win32/gdkwin32.h>
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
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

static gboolean on_gtk_dialog_escape_pressed(GtkWidget* widget, GVariant* args, gpointer win) {
	return on_gtk_window_close_request(widget, win);
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

/*static void on_gtk_window_resize(GtkWidget *widget, gpointer user_data) {
	Window *win = (Window *)user_data;
	Event e = Event("", EventID::RESIZE);
	win->_send_event_(&e);
}*/




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
	requested_destroy = false;

	if ((mode & WIN_MODE_DUMMY) > 0)
		return;

	_init_generic_(_parent, allow_parent, mode);

	// creation
	if (is_dialog()) {
#if GTK_CHECK_VERSION(4,0,0)
		window = gtk_window_new();
#else
		window = gtk_dialog_new();
#endif


		if (!allow_parent)
			gtk_window_set_modal(GTK_WINDOW(window), true);//false);

#if GTK_CHECK_VERSION(4,0,0)
		gtk_widget_insert_action_group(window, "win", G_ACTION_GROUP(action_group));

		shortcut_controller = gtk_shortcut_controller_new();
		gtk_shortcut_controller_set_scope(GTK_SHORTCUT_CONTROLLER(shortcut_controller), GTK_SHORTCUT_SCOPE_GLOBAL);
		gtk_widget_add_controller(window, shortcut_controller);

		// catch [Esc] key
		gtk_shortcut_controller_add_shortcut(
				GTK_SHORTCUT_CONTROLLER(shortcut_controller),
				gtk_shortcut_new(
						gtk_keyval_trigger_new(GDK_KEY_Escape, (GdkModifierType)0),
						gtk_callback_action_new(&on_gtk_dialog_escape_pressed, this, nullptr)));

#endif

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
		}

		//g_action_map_add_action_entries(G_ACTION_MAP(group), entries, G_N_ELEMENTS(entries), this);
		gtk_widget_insert_action_group(window, "win", G_ACTION_GROUP(action_group));

		shortcut_controller = gtk_shortcut_controller_new ();
		gtk_shortcut_controller_set_scope(GTK_SHORTCUT_CONTROLLER(shortcut_controller), GTK_SHORTCUT_SCOPE_GLOBAL);
		gtk_widget_add_controller(window, shortcut_controller);
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
#if GTK_CHECK_VERSION(4,0,0)

	//[[maybe_unused]] auto icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());

	string icon = Application::get_property("icon");
	//msg_write(gtk_icon_theme_has_icon(icon_theme, icon.c_str()));
	gtk_window_set_default_icon_name("tsunami");
	gtk_window_set_icon_name(GTK_WINDOW(window), icon.c_str());
#else
	string logo = Application::get_property("logo");
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

#if !GTK_CHECK_VERSION(4,0,0)
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
#endif

	target_control = nullptr;
}

Window::~Window() {
	DBDEL_START("window", id, this);

	// opengl area might get a render request when deleting the window first
	if (root_control)
		root_control->disable_event_handlers_rec();
	input.reset();

	header_bar = nullptr;

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

namespace WindowFlightManager {
	static shared_array<Window> windows;
	static Array<Window*> destroy_requests;
	void add(shared<Window> win) {
		windows.add(win);
	}
	void remove(Window *win) {
		for (int i=0; i<windows.num; i++)
			if (windows[i] == win)
				windows.erase(i);
		for (int i=0; i<destroy_requests.num; i++)
			if (destroy_requests[i] == win)
				destroy_requests.erase(i);
	}
	bool is_registered(Window *win) {
		for (auto w: weak(windows))
			if (win == w)
				return true;
		return false;
	}
	void request_destroy(Window *win) {
		if (destroy_requests.find(win) >= 0)
			return;
		if (!is_registered(win))
			msg_error("destroying a window that is not registered via fly()");

		destroy_requests.add(win);
		hui::run_later(0.01f, [win] {
			win->end_run_promise();
			remove(win);
			//win->requested_destroy = false;
		});
	}
}

void Window::request_destroy() {
	if (requested_destroy)
		return;
#if GTK_CHECK_VERSION(4,0,0)
	WindowFlightManager::request_destroy(this);
#else
	if (is_dialog())
		gtk_dialog_response(GTK_DIALOG(window), GTK_RESPONSE_DELETE_EVENT);
	else
		WindowFlightManager::request_destroy(this);
#endif
	requested_destroy = true;
}

// should be called after creating (and filling) the window to actually show it
void Window::show() {
	allow_input = true;
#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_set_visible(window, true);
#else
	gtk_widget_show(window);
#endif
}


void on_gtk_window_response(GtkDialog *self, gint response_id, gpointer user_data) {
	auto win = reinterpret_cast<Window*>(user_data);
	win->end_run_promise();
	//gtk_window_destroy(GTK_WINDOW(self));
	run_later(0.01f, [win] { delete win; });
}

void on_gtk_window_response_fly(GtkDialog *self, gint response_id, gpointer user_data) {
	auto win = reinterpret_cast<Window*>(user_data);
	WindowFlightManager::request_destroy(win);
}

base::future<void> fly(shared<Window> win) {
	win->show();

	WindowFlightManager::add(win);

#if GTK_CHECK_VERSION(4,0,0)
	//g_signal_connect(win->window, "response", G_CALLBACK(on_gtk_window_response_fly), win.get());
	gtk_window_present(GTK_WINDOW(win->window));
#else
	if (win->is_dialog())
		g_signal_connect(win->window, "response", G_CALLBACK(on_gtk_window_response_fly), win.get());
#endif

	return win->end_run_promise.get_future();
}

void Window::_wait_till_closed() {
	while (!win->requested_destroy) {
		Application::do_single_main_loop();
		os::sleep(0.005f);
	}
}

void fly_and_wait(shared<Window> win) {
	win->show();
	//int uid = unique_id;

	WindowFlightManager::add(win);

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
		win->_wait_till_closed();
//	}
//#endif
}
class ControlBasicWindowLayout : public Control {
public:
	shared<Toolbar> toolbar[4];
	GtkWidget *hbox = nullptr;
	GtkWidget *inner_box = nullptr;
	GtkWidget *menubar = nullptr;
	Array<GtkWidget*> gtk_menu;
	int gtk_num_menus = 0;
	owned<Menu> menu;
	bool statusbar_enabled = false;
	ControlBasicWindowLayout(const string &title, const string &id, Panel *_panel, const string &layout) :
		Control(CONTROL_BASIC_WINDOW_LAYOUT, id)
	{
		panel = _panel;
		auto win = panel->win;
		auto xx = layout.explode("|");
		bool want_toolbar_top = sa_contains(xx, "toolbar-top") or sa_contains(xx, "toolbar");
		bool want_toolbar_left = sa_contains(xx, "toolbar-left");
		bool want_toolbar_right = sa_contains(xx, "toolbar-right");
		bool want_toolbar_bottom = sa_contains(xx, "toolbar-bottom");
		if (want_toolbar_top)
			toolbar[TOOLBAR_TOP] = new Toolbar(win);
		if (want_toolbar_left)
			toolbar[TOOLBAR_LEFT] = new Toolbar(win, true);
		if (want_toolbar_right)
			toolbar[TOOLBAR_RIGHT] = new Toolbar(win, true);
		if (want_toolbar_bottom)
			toolbar[TOOLBAR_BOTTOM] = new Toolbar(win);

		// vbox
		//   menubar
		//   toolbar top
		//   hbox
		//     toolbar left
		//     inner box
		//     toolbar right
		//   toolbar bottom

		auto vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		inner_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

#if GTK_CHECK_VERSION(4,0,0)

		auto menu = g_menu_new();
		auto m1 = g_menu_new();
		g_menu_append(m1, "test", nullptr);
		//g_menu_append_submenu(menu, "A", G_MENU_MODEL(m1));
		menubar = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(menu));
		gtk_box_append(GTK_BOX(vbox), menubar);

		if (want_toolbar_top)
			gtk_box_append(GTK_BOX(vbox), toolbar[TOOLBAR_TOP]->widget);

		//gtk_container_set_border_width(GTK_CONTAINER(plugable), 0);
		gtk_box_append(GTK_BOX(vbox), hbox);

		if (want_toolbar_left)
			gtk_box_append(GTK_BOX(hbox), toolbar[TOOLBAR_LEFT]->widget);

		gtk_box_append(GTK_BOX(hbox), inner_box);

		if (want_toolbar_right)
			gtk_box_append(GTK_BOX(hbox), toolbar[TOOLBAR_RIGHT]->widget);

		if (want_toolbar_bottom)
			gtk_box_append(GTK_BOX(vbox), toolbar[TOOLBAR_BOTTOM]->widget);
#else

		// menu bar
		menubar = gtk_menu_bar_new();
		gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

		// tool bars
		if (want_toolbar_top) {
			gtk_style_context_add_class(gtk_widget_get_style_context(toolbar[TOOLBAR_TOP]->widget), "primary-toolbar");
			gtk_box_pack_start(GTK_BOX(vbox), toolbar[TOOLBAR_TOP]->widget, FALSE, FALSE, 0);
		}

		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
		//gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
		gtk_widget_show(hbox);

		if (want_toolbar_left)
			gtk_box_pack_start(GTK_BOX(hbox), toolbar[TOOLBAR_LEFT]->widget, FALSE, FALSE, 0);

		gtk_widget_show(inner_box);
		//gtk_container_set_border_width(GTK_CONTAINER(inner_box), 0);
		gtk_box_pack_start(GTK_BOX(hbox), inner_box, TRUE, TRUE, 0);

		if (want_toolbar_right)
			gtk_box_pack_start(GTK_BOX(hbox), toolbar[TOOLBAR_RIGHT]->widget, FALSE, FALSE, 0);
		if (want_toolbar_bottom)
			gtk_box_pack_start(GTK_BOX(vbox), toolbar[TOOLBAR_BOTTOM]->widget, FALSE, FALSE, 0);
#endif
		widget = vbox;
		take_gtk_ownership();
	}

	void add_child(shared<Control> child, int x, int y) override {
		auto child_widget = child->get_frame();
#if GTK_CHECK_VERSION(4,0,0)
		gtk_box_append(GTK_BOX(inner_box), child_widget);
#else
		gtk_box_pack_start(GTK_BOX(inner_box), child_widget, true, true, 0);
#endif
		control_link(this, child);

		DBDEL_X(format("BasicLayout.add  %s  cw=%s", child->id, p2s(child->widget)));
	}


	void remove_child(Control *child) override {
		DBDEL_X(format("BasicLayout.remove  %s  cw=%s", child->id, p2s(child->widget)));
		auto child_widget = child->get_frame();
	#if GTK_CHECK_VERSION(4,0,0)
		gtk_box_remove(GTK_BOX(widget), child_widget);
	#else
		gtk_container_remove(GTK_CONTAINER(widget), child_widget);
	#endif
		control_unlink(this, child);
	}

	void set_menu(xfer<Menu> _menu) {

#if GTK_CHECK_VERSION(4,0,0)
	//action_group = g_simple_action_group_new();

	if (_menu) {
		menu = _menu;

		panel->_connect_menu_to_panel(menu.get());

		gtk_popover_menu_bar_set_menu_model(GTK_POPOVER_MENU_BAR(menubar), G_MENU_MODEL(menu->gmenu));
		gtk_widget_set_visible(menubar, true);
	} else {
		menu = _menu;
		auto dummy = g_menu_new();
		gtk_popover_menu_bar_set_menu_model(GTK_POPOVER_MENU_BAR(menubar), G_MENU_MODEL(dummy));
		gtk_widget_set_visible(menubar, false);
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
		menu = nullptr;
	}

	
	// insert new menu
	menu = _menu;
	if (menu) {
		menu->set_panel(panel->win);
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
#if GTK_CHECK_VERSION(4,0,0)
		gtk_widget_set_visible(menubar, false);
#else
		gtk_widget_hide(menubar);
#endif
	}
#endif
	}

};


void Window::set_menu(xfer<Menu> _menu) {
	if (!basic_layout)
		return;
	basic_layout->set_menu(_menu);
}

int key_to_gtk(int key_code, GdkModifierType* mod) {

	int _mod = (((key_code & KEY_SHIFT)>0) ? GDK_SHIFT_MASK : 0) |
				(((key_code & KEY_CONTROL)>0) ? GDK_CONTROL_MASK : 0) |
#if GTK_CHECK_VERSION(4,0,0)
				(((key_code & KEY_ALT)>0) ? GDK_ALT_MASK : 0) |
#else
				(((key_code & KEY_ALT)>0) ? GDK_MOD1_MASK : 0) |
#endif
				(((key_code & KEY_META)>0) ? GDK_META_MASK : 0) |
				(((key_code & KEY_SUPER)>0) ? GDK_SUPER_MASK : 0) |
				(((key_code & KEY_HYPER)>0) ? GDK_HYPER_MASK : 0);
	*mod = (GdkModifierType)_mod;
	return HuiKeyID[key_code & 255];
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

	GdkModifierType mod;
	int gtk_key = key_to_gtk(key_code, &mod);
	gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(shortcut_controller),
		gtk_shortcut_new(gtk_keyval_trigger_new(gtk_key, mod), gtk_named_action_new(get_gtk_action_name(id, this).c_str())));
#endif
}

void Window::add_action_checkable(const string &id) {
#if GTK_CHECK_VERSION(4,0,0)
	_try_add_action_(id, true);
#endif
}

// show/hide without closing the window
void Window::hide() {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_set_visible(window, false);
#else
	gtk_widget_hide(window);
#endif
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
	width = gtk_widget_get_width(GTK_WIDGET(window));
	height = gtk_widget_get_height(GTK_WIDGET(window));
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
#if GTK_CHECK_VERSION(4,0,0)
	static bool complained = false;
	if (!complained) {
		msg_error("hui.Window.show_cursor() not implemented for gtk4");
		complained = true;
	}
#else
	// TODO use root control
	if (basic_layout) {
		if (show)
			gdk_window_set_cursor(gtk_widget_get_window(basic_layout->widget), nullptr);
		else
			gdk_window_set_cursor(gtk_widget_get_window(basic_layout->widget), (GdkCursor*)invisible_cursor);
	}
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
		} else if (op == "closebutton") {
			if (header_bar)
				header_bar->set_options(op + "=" + val);
		} else if (op == "layout") {
			add_basic_layout(val);
		} else if (op == "cursor") {
			show_cursor(val_is_positive(val, true));
		} else if (op == "borderwidth") {
			set_border_width(val._int());
		} else if (op == "spacing") {
			spacing = val._int();
		}
	}
}

Toolbar *Window::get_toolbar(int index) {
	if (!basic_layout)
		return nullptr;
	return basic_layout->toolbar[index].get();
}

Menu *Window::get_menu() {
	if (!basic_layout)
		return nullptr;
	return basic_layout->menu.get();
}

void Window::add_basic_layout(const string& l) {
	basic_layout = new ControlBasicWindowLayout("", ":layout:", this, l);
	_insert_control_(basic_layout, 0, 0);
	target_control = basic_layout;
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
