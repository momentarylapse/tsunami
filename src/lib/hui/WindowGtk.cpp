#include "Controls/Control.h"
#include "hui.h"
#include "internal.h"
#include "Toolbar.h"
#ifdef HUI_API_GTK


#ifdef OS_WINDOWS
	#include <gdk/gdkwin32.h>
#endif
#ifdef OS_LINUX
#if HAS_LIB_XLIB
	#include <gdk/gdkx.h>
#endif
#endif

namespace hui
{


void DBDEL(const string &type, const string &id, void *p);
void DBDEL_DONE();


#if !GTK_CHECK_VERSION(3,0,0)
GtkWidget *gtk_box_new(GtkOrientation orientation, int spacing); // -> hui_window_control_gtk.cpp
#endif


unsigned int ignore_time = 0;


//----------------------------------------------------------------------------------
// window message handling

inline Window *win_from_widget(void *widget)
{
	for (Window *win: _all_windows_)
		if (win->window == widget)
			return win;
	return nullptr;
}

void WinTrySendByKeyCode(Window *win, int key_code)
{
	if (key_code <= 0)
		return;
	for (auto &k: win->event_key_codes)
		if (key_code == k.key_code){
			Event e = Event(k.id, k.message);
			win->_send_event_(&e);
		}
}

static gboolean on_gtk_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	Window *win = (Window *)user_data;
	Event e = Event("", "hui:close");
	if (win->_send_event_(&e))
		return true;
	win->on_close_request();
	return true;
}

static gboolean on_gtk_window_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	Window *win = (Window *)user_data;
	// make sure the contro/alt/shift keys are unset

	// reset all keys
	memset(&win->input.key, 0, sizeof(win->input.key));
	/*win->input.key[KEY_RSHIFT] = win->input.key[KEY_LSHIFT] = false;
	win->input.key[KEY_RCONTROL] = win->input.key[KEY_LCONTROL] = false;
	win->input.key[KEY_RALT] = win->input.key[KEY_LALT] = false;*/
	return false;
}

static void on_gtk_window_resize(GtkWidget *widget, gpointer user_data)
{
	Window *win = (Window *)user_data;
	Event e = Event("", "hui:resize");
	win->_send_event_(&e);
}




//----------------------------------------------------------------------------------
// window functions


// general window

void Window::_init_(const string &title, int width, int height, Window *parent, bool allow_parent, int mode)
{
	window = nullptr;
	win = this;
	if ((mode & WIN_MODE_DUMMY) > 0)
		return;

	_init_generic_(parent, allow_parent, mode);

	// creation
	if (parent){
		//window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		//gtk_window_set_modal(GTK_WINDOW(window), !allow_parent);
		
		window = gtk_dialog_new();
		if (!allow_parent)
			gtk_window_set_modal(GTK_WINDOW(window), true);//false);
		// TODO GTK3
		//gtk_dialog_set_has_separator(GTK_DIALOG(window), false);
		//gtk_container_set_border_width(GTK_CONTAINER(window), 0);
#ifndef OS_WINDOWS
#if !GTK_CHECK_VERSION(3,0,0)
		gtk_widget_hide(gtk_dialog_get_action_area(GTK_DIALOG(window)));
#elif !GTK_CHECK_VERSION(3,12,0)
		gtk_widget_hide(gtk_dialog_get_action_area(GTK_DIALOG(window)));
#endif
#endif
	}else
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	GetPartStrings(title);

	gtk_window_set_title(GTK_WINDOW(window), sys_str(PartString[0]));
	gtk_window_set_resizable(GTK_WINDOW(window), true);
	if (parent){
		// dialog -> center on screen or root (if given)    ->  done by gtk....later
		/*if (parent){
			irect r=parent->GetOuterior();
			x = r.x1 + (r.x2-r.x1-width)/2;
			y = r.y1 + (r.y2-r.y1-height)/2;
		}else{
			GdkScreen *screen=gtk_window_get_screen(GTK_WINDOW(window));
			x=(gdk_screen_get_width(screen)-width)/2;
			y=(gdk_screen_get_height(screen)-height)/2;
		}
		//gtk_window_move(GTK_WINDOW(window),x,y);*/
		gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent->window));
	}else{
		/*if ((x >= 0) && (y >= 0))
			gtk_window_move(GTK_WINDOW(window), x, y);*/
	}

	// size
	gtk_window_resize(GTK_WINDOW(window), width, height);
	desired_width = width;
	desired_height = height;

	// icon
	string logo = Application::get_property("logo");
	if (logo.num > 0)
		gtk_window_set_icon_from_file(GTK_WINDOW(window), sys_str_f(logo), nullptr);

	// catch signals
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(&on_gtk_window_close), this);
	g_signal_connect(G_OBJECT(window), "focus-in-event", G_CALLBACK(&on_gtk_window_focus), this);
	//g_signal_connect(G_OBJECT(window), "state-flags-changed", G_CALLBACK(&on_gtk_window_resize), this);

	// fill in some stuff
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
	if (parent){
		vbox = gtk_dialog_get_content_area(GTK_DIALOG(window));
	}else{
		vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		//gtk_box_pack_start(GTK_BOX(window), vbox, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(window), vbox);
		gtk_widget_show(vbox);
	}

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
	cur_control = nullptr;
	// free to use...
	//cur_control = hbox;
	plugable = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(plugable);
	//gtk_container_set_border_width(GTK_CONTAINER(plugable), 0);
	gtk_box_pack_start(GTK_BOX(hbox), plugable, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), toolbar[TOOLBAR_RIGHT]->widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar[TOOLBAR_BOTTOM]->widget, FALSE, FALSE, 0);

	// status bar
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);
	
#ifdef OS_WINDOWS
	hWnd = nullptr;
#endif
}

Window::~Window()
{
	if (!window)
		return;

	destroy();
}

void Window::__delete__()
{
	this->Window::~Window();
}

void Window::destroy()
{
	DBDEL("window", id, this);
	on_destroy();

	_clean_up_();

	gtk_widget_destroy(window);
	window = nullptr;
	DBDEL_DONE();
}

bool Window::got_destroyed()
{
	return window == nullptr;
}

// should be called after creating (and filling) the window to actually show it
void Window::show()
{
	allow_input = true;
	gtk_widget_show(window);
#ifdef OS_WINDOWS
	hWnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(window));
#endif
}

void Window::run()
{
	show();
	int uid = unique_id;
	/*msg_write((int)win);
	msg_write(win->uid);*/
	string last_id = "";

#ifdef HUI_API_WIN
	MSG messages;
	messages.message=0;
	bool got_message;
	//while ((WM_QUIT!=messages.message)&&(!WindowClosed[win_no])){
	while (WM_QUIT!=messages.message){
		bool br=false;
		for (int i=0;i<_HuiClosedWindow_.size();i++)
			if (_HuiClosedWindow_[i].UID==uid)
				br=true;
		if (br)
			break;
		bool allow=true;
		if (HuiIdleFunction)
			got_message=(PeekMessage(&messages,nullptr,0U,0U,PM_REMOVE)!=0);
		else
			got_message=(GetMessage(&messages,nullptr,0,0)!=0);
		if (got_message){
			allow=false;
			TranslateMessage(&messages);
			DispatchMessage(&messages);
		}
		if ((HuiIdleFunction)&&(allow))
			HuiIdleFunction();
	}
	if (WM_QUIT==messages.message){
		HuiHaveToExit=true;
		//msg_write("EXIT!!!!!!!!!!");
	}
#endif
#ifdef HUI_API_GTK
	if (get_parent()){
		gtk_dialog_run(GTK_DIALOG(window));
	}else{
		while(!got_destroyed()){
			Application::do_single_main_loop();
			Sleep(0.005f);
		}
	}
#endif
}

void Window::set_menu(Menu *_menu)
{
	// remove old menu...
	if (menu){
		Array<Control*> list = menu->get_all_controls();
		// move items from <menu_bar> back to <Menu>
		for (int i=0;i<gtk_menu.num;i++){
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
	if (menu){
		menu->set_panel(this);
		gtk_widget_show(menubar);
		gtk_num_menus = menu->items.num;
		for (int i=0;i<menu->items.num;i++){
			// move items from <Menu> to <menu_bar>
			Control *it = menu->items[i];
			gtk_menu.add(it->widget);
			gtk_widget_show(gtk_menu[i]);
			g_object_ref(it->widget);
			gtk_container_remove(GTK_CONTAINER(menu->widget), gtk_menu[i]);
			gtk_menu_shell_append(GTK_MENU_SHELL(menubar), gtk_menu[i]);
			g_object_unref(it->widget);
		}
	}else
		gtk_widget_hide(menubar);
}

// show/hide without closing the window
void Window::hide()
{
	gtk_widget_hide(window);
}

// set the string in the title bar
void Window::set_title(const string &title)
{
	gtk_window_set_title(GTK_WINDOW(window),sys_str(title));
}

// set the upper left corner of the window in screen corrdinates
void Window::set_position(int x, int y)
{
	gtk_window_move(GTK_WINDOW(window),x,y);
}

void Window::get_position(int &x, int &y)
{
	gtk_window_get_position(GTK_WINDOW(window), &x, &y);
}

void Window::set_size(int width, int height)
{
	desired_width = width;
	desired_height = height;
	/*if (parent)
		gtk_widget_set_size_request(window, width, height);
	else*/
		gtk_window_resize(GTK_WINDOW(window), width, height);
}

// get the current window position and size (including the frame and menu/toolbars...)
void Window::get_size(int &width, int &height)
{
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
}

// set the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <SetOuterior>
void Window::set_size_desired(int width, int height)
{
	// bad hack
	bool maximized = is_maximized();
	if (maximized)
		gtk_window_unmaximize(GTK_WINDOW(window));
	gtk_window_resize(GTK_WINDOW(window), width, height);
	if (maximized)
		gtk_window_maximize(GTK_WINDOW(window));
	desired_width = width;
	desired_height = height;
}

// get the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <GetOuterior>
void Window::get_size_desired(int &width, int &height)
{
	if (is_maximized()){
		width = desired_width;
		height = desired_height;
	}else{
		gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	}
}

void Window::show_cursor(bool show)
{
#ifdef OS_WINDOWS
	int s=::ShowCursor(show);
	if (show){
		while(s<0)
			s=::ShowCursor(show);
	}else{
		while(s>=0)
			s=::ShowCursor(show);
	}
#else
	if (show)
		gdk_window_set_cursor(gtk_widget_get_window(vbox), nullptr);
	else
		gdk_window_set_cursor(gtk_widget_get_window(vbox), (GdkCursor*)invisible_cursor);
#endif
}

extern int GtkAreaMouseSet;
extern int GtkAreaMouseSetX, GtkAreaMouseSetY;

// relative to Interior
void Window::set_cursor_pos(int x, int y)
{
	if (main_input_control){
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
}

void Window::set_maximized(bool maximized)
{
	if (maximized){
		if (!is_maximized())
			gtk_window_get_size(GTK_WINDOW(window), &desired_width, &desired_height);
		gtk_window_maximize(GTK_WINDOW(window));
	}else{
		gtk_window_unmaximize(GTK_WINDOW(window));
	}
}

bool Window::is_maximized()
{
	return gtk_window_is_maximized(GTK_WINDOW(window));
}

bool Window::is_minimized()
{
	int state = gdk_window_get_state(gtk_widget_get_window(window));
	return ((state & GDK_WINDOW_STATE_ICONIFIED) > 0);
}

void Window::set_fullscreen(bool fullscreen)
{
	if (fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(window));
}

void Window::enable_statusbar(bool enabled)
{
	if (enabled)
	    gtk_widget_show(statusbar);
	else
	    gtk_widget_hide(statusbar);
	statusbar_enabled = enabled;
}

void Window::set_status_text(const string &str)
{
	gtk_statusbar_push(GTK_STATUSBAR(statusbar),0,sys_str(str));
}

static Array<string> __info_bar_responses;
static int make_info_bar_response(const string &id)
{
	foreachi (string &_id, __info_bar_responses, i)
		if (_id == id)
			return i + 1234;
	__info_bar_responses.add(id);
	return __info_bar_responses.num - 1 + 1234;
}

void __GtkOnInfoBarResponse(GtkWidget *widget, int response, gpointer data)
{
	gtk_widget_destroy(widget);
	Window *win = (Window*)data;

	int index = response - 1234;
	if (index >= 0 and index < __info_bar_responses.num){
		Event e = Event(__info_bar_responses[index], "hui:info");
		win->_send_event_(&e);
	}
	//win->infobar = nullptr;
	//win->
}

Window::InfoBar *Window::_get_info_bar(const string &id) {
	for (auto &i: info_bars)
		if (i.id == id) {
			gtk_widget_destroy(i.widget);
			return &i;
		}


	InfoBar i;
	i.id = id;
	info_bars.add(i);
	return &info_bars.back();
}

void Window::set_info_text(const string &str, const Array<string> &options) {
	string id = "default";
	for (string &o: options)
		if (o.head(3) == "id=")
			id = o.substr(3, -1);

	auto infobar = _get_info_bar(id);

	infobar->widget = gtk_info_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), infobar->widget, FALSE, FALSE, 0);
	gtk_box_reorder_child(GTK_BOX(vbox), infobar->widget, 2);
	//gtk_widget_set_no_show_all(infobar, TRUE);

	auto *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR(infobar->widget));
	infobar->label = gtk_label_new("");
	//gtk_label_set_text(GTK_LABEL (message_label), sys_str(str));
	gtk_container_add(GTK_CONTAINER(content_area), infobar->label);

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
		if (o.head(7) == "button:"){
			auto x = o.explode(":");
			if (x.num >= 3)
				gtk_info_bar_add_button(GTK_INFO_BAR(infobar->widget), sys_str(x[2]), make_info_bar_response(x[1]));
		}
	}
	gtk_info_bar_set_message_type(GTK_INFO_BAR(infobar->widget), type);
	gtk_info_bar_set_show_close_button(GTK_INFO_BAR(infobar->widget), allow_close);

	gtk_widget_show(infobar->widget);
	gtk_widget_show(infobar->label);
}


// give our window the focus....and try to focus the specified control item
void Panel::activate(const string &control_id)
{
	if (win) {
		gtk_widget_grab_focus(win->window);
		gtk_window_present(GTK_WINDOW(win->window));
	}
	if (control_id.num > 0)
		apply_foreach(control_id, [=](Control *c){ c->focus(); });
}

bool Panel::is_active(const string &control_id)
{
	if (control_id.num > 0){
		bool r = false;
		apply_foreach(control_id, [&](Control *c){ r = c->has_focus(); });
		return r;
	}
	if (!win)
		return false;
	return (bool)gtk_widget_has_focus(win->window);
}

};


#endif
