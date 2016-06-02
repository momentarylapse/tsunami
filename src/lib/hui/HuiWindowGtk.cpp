#include "hui.h"
#include "hui_internal.h"
#include "HuiToolbar.h"
#include "Controls/HuiControl.h"
#ifdef HUI_API_GTK


#ifdef OS_WINDOWS
	#include <gdk/gdkwin32.h>
#endif
#ifdef OS_LINUX
	#include <gdk/gdkx.h>
#endif


void add_key_to_buffer(HuiInputData *d, int key);

#if !GTK_CHECK_VERSION(3,0,0)
GtkWidget *gtk_box_new(GtkOrientation orientation, int spacing); // -> hui_window_control_gtk.cpp
#endif


unsigned int ignore_time = 0;


//----------------------------------------------------------------------------------
// window message handling

inline HuiWindow *win_from_widget(void *widget)
{
	for (int i=0;i<HuiWindows.num;i++)
		if (HuiWindows[i]->window == widget)
			return HuiWindows[i];
	return NULL;
}

#if 0
bool win_send_message(GtkWidget *widget, const string &message, gpointer user_data)
{
	HuiWindow *window = (HuiWindow*)user_data;
	if (!window)
		return false;
	HuiCurWindow = window;
	if (widget == window->window){
		//if (gtk_window_has_toplevel_focus(GTK_WINDOW(window->window))){
		//_so("top_level");
		if (window->AllowInput){
			window->_send_event_(message, "", NULL);
			return true;
		}
	}else{
/*		//if (gtk_window_has_toplevel_focus(GTK_WINDOW(window->window))){
		//_so("top_level");
		if (HuiRunning)
		for (int i=0;i<window->Control.num;i++)
			if (window->Control[i]->win == widget){
				if (window->Control[i]->input_handler){
					sHuiInputMessage m;
					m.message = message;
					m.id = window->Control[i]->ID;
					int mod;
					gdk_window_get_pointer(widget->window, &m.mx, &m.my, (GdkModifierType*)&mod);
					gdk_drawable_get_size(window->Control[i]->win->window, &m.width, &m.height);

					m.lbut = ((mod & GDK_BUTTON1_MASK) > 0);
					m.mbut = ((mod & GDK_BUTTON2_MASK) > 0);
					m.rbut = ((mod & GDK_BUTTON3_MASK) > 0);
					window->Control[i]->input_handler(&m);
					return true;
				}
			}*/
	}
	return false;
}
#endif


void WinTrySendByKeyCode(HuiWindow *win, int key_code)
{
	if (key_code <= 0)
		return;
	for (HuiCommand &c : _HuiCommand_)
		if (key_code == c.key_code){
			//msg_write("---------------------------------");
			//msg_write(c.id);
			HuiEvent e = HuiEvent(c.id, "");
			_HuiSendGlobalCommand_(&e);
			win->_send_event_(&e);
		}
}

gboolean OnGtkWindowClose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	HuiWindow *win = (HuiWindow *)user_data;
	HuiEvent e = HuiEvent("", "hui:close");
	if (win->_send_event_(&e))
		return true;
	win->onCloseRequest();
	return true;
}

gboolean OnGtkWindowFocus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	HuiWindow *win = (HuiWindow *)user_data;
	// make sure the contro/alt/shift keys are unset

	// reset all keys
	memset(&win->input.key, 0, sizeof(win->input.key));
	/*win->input.key[KEY_RSHIFT] = win->input.key[KEY_LSHIFT] = false;
	win->input.key[KEY_RCONTROL] = win->input.key[KEY_LCONTROL] = false;
	win->input.key[KEY_RALT] = win->input.key[KEY_LALT] = false;*/
	return false;
}




//----------------------------------------------------------------------------------
// window functions


// general window

void HuiWindow::_init_(const string &title, int x, int y, int width, int height, HuiWindow *root, bool allow_root, int mode)
{
	msg_db_f("HuiWindow()",1);

	window = NULL;
	win = this;
	if ((mode & HuiWinModeDummy) > 0)
		return;

	_init_generic_(root, allow_root, mode);
	
	bool ControlMode = ((mode & HuiWinModeControls) > 0);

	// creation
	if (parent){
		//window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		//gtk_window_set_modal(GTK_WINDOW(window), !allow_root);
		
		window = gtk_dialog_new();
		if (!allow_root)
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

	gtk_window_set_title(GTK_WINDOW(window), sys_str(title));
	gtk_window_set_resizable(GTK_WINDOW(window), is_resizable);
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
		if ((x >= 0) && (y >= 0))
			gtk_window_move(GTK_WINDOW(window), x, y);
	}

	// size
	if (is_resizable)
		gtk_window_resize(GTK_WINDOW(window), width, height);
	else
		gtk_widget_set_size_request(window, width, height);
	desired_width = width;
	desired_height = height;

	// icon
	string logo = HuiGetProperty("logo");
	if (logo.num > 0)
		gtk_window_set_icon_from_file(GTK_WINDOW(window), sys_str_f(logo), NULL);

	// catch signals
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(&OnGtkWindowClose), this);
	g_signal_connect(G_OBJECT(window), "focus-in-event", G_CALLBACK(&OnGtkWindowFocus), this);

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
	gtk_style_context_add_class(gtk_widget_get_style_context(toolbar[HuiToolbarTop]->widget), "primary-toolbar");
#endif
	gtk_box_pack_start(GTK_BOX(vbox), toolbar[HuiToolbarTop]->widget, FALSE, FALSE, 0);


	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	//gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	gtk_box_pack_start(GTK_BOX(hbox), toolbar[HuiToolbarLeft]->widget, FALSE, FALSE, 0);

	plugable = NULL;
	cur_control = NULL;
	if (ControlMode){
		if (is_resizable){
			// free to use...
			//cur_control = hbox;
			plugable = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_widget_show(plugable);
			//gtk_container_set_border_width(GTK_CONTAINER(plugable), 0);
			gtk_box_pack_start(GTK_BOX(hbox), plugable, TRUE, TRUE, 0);
		}else{
			// "fixed" (for buttons etc)
			plugable = gtk_fixed_new();
			gtk_box_pack_start(GTK_BOX(hbox), plugable, TRUE, TRUE, 0);
			gtk_widget_show(plugable);
			//gl_widget = plugable;
		}
	}

	gtk_box_pack_start(GTK_BOX(hbox), toolbar[HuiToolbarRight]->widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar[HuiToolbarBottom]->widget, FALSE, FALSE, 0);

	// status bar
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);
	
#ifdef OS_WINDOWS
	hWnd = NULL;
#endif
}

HuiWindow::~HuiWindow()
{
	msg_db_f("HuiWindow.~", 1);

	if (!window)
		return;

	// quick'n'dirty fix (gtk destroys its widgets recursively)
	for (HuiControl *c : control)
		c->widget = NULL;

	_clean_up_();

	gtk_widget_destroy(window);
}

void HuiWindow::__delete__()
{
	this->HuiWindow::~HuiWindow();
}

// should be called after creating (and filling) the window to actually show it
void HuiWindow::show()
{
	gtk_widget_show(window);
#ifdef OS_WINDOWS
	hWnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(window));
#endif

	allow_input = true;
}

string HuiWindow::run()
{
	msg_db_f("HuiWindow.Run",1);
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
			got_message=(PeekMessage(&messages,NULL,0U,0U,PM_REMOVE)!=0);
		else
			got_message=(GetMessage(&messages,NULL,0,0)!=0);
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
	if (getParent()){
		gtk_dialog_run(GTK_DIALOG(window));
	}else{
		bool killed = false;
		while(!killed){
			HuiDoSingleMainLoop();
			for (HuiClosedPanel &cp : HuiClosedPanels)
				if (cp.unique_id == uid)
					killed = true;
		}
	}
#endif
	//msg_write("cleanup");

	// clean up
	foreachi(HuiClosedPanel &cp, HuiClosedPanels, i)
		if (cp.unique_id == uid){
			if (cp.last_id.num > 0)
				last_id = cp.last_id;
			HuiClosedPanels.erase(i);
		}
	return last_id;
}

void HuiWindow::setMenu(HuiMenu *_menu)
{
	msg_db_f("SetMenu", 1);
	// remove old menu...
	if (menu){
		Array<HuiControl*> list = menu->get_all_controls();
		// move items from <menu_bar> back to <Menu>
		for (int i=0;i<gtk_menu.num;i++){
			g_object_ref(gtk_menu[i]);
			gtk_container_remove(GTK_CONTAINER(menubar), GTK_WIDGET(gtk_menu[i]));
			gtk_menu_shell_append(GTK_MENU_SHELL(menu->widget), GTK_WIDGET(gtk_menu[i]));
			g_object_unref(gtk_menu[i]);
		}
		gtk_menu.clear();
		/*menu->set_win(NULL);
		foreach(HuiControl *c, list){
			for (int i=0;i<control.num;i++)
				if (control[i] == c)
					control.erase(i);
		}*/
		delete(menu);
	}

	
	// insert new menu
	menu = _menu;
	if (menu){
		menu->set_panel(this);
		gtk_widget_show(menubar);
		gtk_num_menus = menu->item.num;
		for (int i=0;i<menu->item.num;i++){
			// move items from <Menu> to <menu_bar>
			HuiControl *it = menu->item[i];
			gtk_menu.add(it->widget);
			gtk_widget_show(gtk_menu[i]);
			g_object_ref(it->widget);
			gtk_container_remove(GTK_CONTAINER(menu->widget), gtk_menu[i]);
			gtk_menu_shell_append(GTK_MENU_SHELL(menubar), gtk_menu[i]);
			g_object_unref(it->widget);
		}
		Array<HuiControl*> list = menu->get_all_controls();
		control.append(list);
	}else
		gtk_widget_hide(menubar);
}

// show/hide without closing the window
void HuiWindow::hide()
{
	gtk_widget_hide(window);
}

// set the string in the title bar
void HuiWindow::setTitle(const string &title)
{
	gtk_window_set_title(GTK_WINDOW(window),sys_str(title));
}

// set the upper left corner of the window in screen corrdinates
void HuiWindow::setPosition(int x, int y)
{
	gtk_window_move(GTK_WINDOW(window),x,y);
}

void HuiWindow::getPosition(int &x, int &y)
{
	gtk_window_get_position(GTK_WINDOW(window), &x, &y);
}

void HuiWindow::setSize(int width, int height)
{
	desired_width = width;
	desired_height = height;
	if (parent)
		gtk_widget_set_size_request(window, width, height);
	else
		gtk_window_resize(GTK_WINDOW(window), width, height);
}

// get the current window position and size (including the frame and menu/toolbars...)
void HuiWindow::getSize(int &width, int &height)
{
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
}

// set the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <SetOuterior>
void HuiWindow::setSizeDesired(int width, int height)
{
	// bad hack
	bool maximized = isMaximized();
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
void HuiWindow::getSizeDesired(int &width, int &height)
{
	if (isMaximized()){
		width = desired_width;
		height = desired_height;
	}else{
		gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	}
}

void HuiWindow::showCursor(bool show)
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
		gdk_window_set_cursor(gtk_widget_get_window(vbox), NULL);
	else
		gdk_window_set_cursor(gtk_widget_get_window(vbox), (GdkCursor*)invisible_cursor);
#endif
}

extern int GtkAreaMouseSet;
extern int GtkAreaMouseSetX, GtkAreaMouseSetY;

// relative to Interior
void HuiWindow::setCursorPos(int x, int y)
{
	if (main_input_control){
		//msg_write(format("set cursor %d %d  ->  %d %d", (int)input.x, (int)input.y, x, y));
		GtkAreaMouseSet = 2;
		GtkAreaMouseSetX = x;
		GtkAreaMouseSetY = y;
		input.x = (float)x;
		input.y = (float)y;
		// TODO GTK3
#ifdef OS_LINUX
		XWarpPointer(hui_x_display, None, GDK_WINDOW_XID(gtk_widget_get_window(main_input_control->widget)), 0, 0, 0, 0, x, y);
		XFlush(hui_x_display);
#endif
#ifdef OS_WINDOWS
		RECT r;
		GetWindowRect((HWND)GDK_WINDOW_HWND(gtk_widget_get_window(main_input_control->widget)), &r);
		::SetCursorPos(x + r.left, y + r.top);
#endif
	}
}

void HuiWindow::setMaximized(bool maximized)
{
	if (maximized){
		if (!isMaximized())
			gtk_window_get_size(GTK_WINDOW(window), &desired_width, &desired_height);
		gtk_window_maximize(GTK_WINDOW(window));
	}else{
		gtk_window_unmaximize(GTK_WINDOW(window));
	}
}

bool HuiWindow::isMaximized()
{
	int state = gdk_window_get_state(gtk_widget_get_window(window));
	return ((state & GDK_WINDOW_STATE_MAXIMIZED) > 0);
}

bool HuiWindow::isMinimized()
{
	int state = gdk_window_get_state(gtk_widget_get_window(window));
	return ((state & GDK_WINDOW_STATE_ICONIFIED) > 0);
}

void HuiWindow::setFullscreen(bool fullscreen)
{
	if (fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(window));
}

void HuiWindow::enableStatusbar(bool enabled)
{
	if (enabled)
	    gtk_widget_show(statusbar);
	else
	    gtk_widget_hide(statusbar);
	statusbar_enabled = enabled;
}

void HuiWindow::setStatusText(const string &str)
{
	gtk_statusbar_push(GTK_STATUSBAR(statusbar),0,sys_str(str));
}


// give our window the focus....and try to focus the specified control item
void HuiPanel::activate(const string &control_id)
{
	gtk_widget_grab_focus(win->window);
	gtk_window_present(GTK_WINDOW(win->window));
	if (control_id.num > 0)
		for (int i=0;i<control.num;i++)
			if (control_id == control[i]->id)
				control[i]->focus();
}

bool HuiPanel::isActive(const string &control_id)
{
	if (control_id.num > 0){
		for (int i=0;i<control.num;i++)
			if (control_id == control[i]->id)
				return control[i]->hasFocus();
		return false;
	}
	return (bool)gtk_widget_has_focus(win->window);
}




#endif
