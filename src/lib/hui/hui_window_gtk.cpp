#include "hui.h"
#include "hui_internal.h"
#ifdef HUI_API_GTK


#ifdef OS_WINDOWS
	#include <gdk/gdkwin32.h>
#endif
#ifdef OS_LINUX
	#include <gdk/gdkx.h>
#endif


void add_key_to_buffer(HuiInputData *d, int key);

#if GTK_MAJOR_VERSION == 2
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
			window->_SendEvent_(message, "", NULL);
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
	foreach(HuiCommand &c, _HuiCommand_)
		if (key_code == c.key_code){
			//msg_write("---------------------------------");
			//msg_write(c.id);
			HuiEvent e = HuiEvent(c.id, "");
			_HuiSendGlobalCommand_(&e);
			win->_SendEvent_(&e);
		}
}

void _get_hui_key_id_(GdkEventKey *event, int &key, int &key_code)
{
	// convert hardware keycode into GDK keyvalue
	GdkKeymapKey kmk;
	kmk.keycode = event->hardware_keycode;
	kmk.group = event->group;
	kmk.level = 0;
	int keyvalue = gdk_keymap_lookup_key(gdk_keymap_get_default(), &kmk);
	// TODO GTK3
	//int keyvalue = event->keyval;
	//msg_write(keyvalue);

	// convert GDK keyvalue into HUI key id
	key = -1;
	for (int i=0;i<HUI_NUM_KEYS;i++)
		//if ((HuiKeyID[i] == keyvalue)||(HuiKeyID2[i] == keyvalue))
		if (HuiKeyID[i] == keyvalue)
			key = i;
	key_code = key;
	if (key < 0)
		return;


	// key code?
	if ((event->state & GDK_CONTROL_MASK) > 0)
		key_code += KEY_CONTROL;
	if ((event->state & GDK_SHIFT_MASK) > 0)
		key_code += KEY_SHIFT;
	if (((event->state & GDK_MOD1_MASK) > 0) /*|| ((event->state & GDK_MOD2_MASK) > 0) || ((event->state & GDK_MOD5_MASK) > 0)*/)
		key_code += KEY_ALT;
}

bool process_key(GdkEventKey *event, GtkWidget *KeyReciever, HuiWindow *win, bool down)
{
	//printf("%d   %d\n", nk++, event->time);
	int key, key_code;
	_get_hui_key_id_(event, key, key_code);
	if (key < 0){
		//msg_write(format("unknown key: %d", event->hardware_keycode));
		return false;
	}

/*#ifdef OS_LINUX
	char key_map_stat[32];
	XQueryKeymap(hui_x_display, key_map_stat);
	bool actual_state = ((key_map_stat[event->hardware_keycode >> 3] >> (event->hardware_keycode & 7)) & 1);
#else
	bool actual_state = down;
#endif*/

	if (win->allow_input)
		win->input.key[key] = down;//actual_state;

	//printf("%d\n", event->state & GDK_CONTROL_MASK);

	win->input.key_code = key;

	msg_db_m(format("%s:  %s", down ? "down" : "up", HuiGetKeyName(key).c_str()).c_str(), 1);
	//if ((down) && (win->input.key[key])){
	if (down){
		add_key_to_buffer(&win->input, key);

		win->input.key_code = key_code;
		WinTrySendByKeyCode(win, key_code);
	}

	HuiEvent e = HuiEvent("", down ? "hui:key-down" : "hui:key-up");
	win->_SendEvent_(&e);
	
	return true;
}

gboolean OnGtkWindowClose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	HuiWindow *win = (HuiWindow *)user_data;
	HuiEvent e = HuiEvent("", "hui:close");
	if (win->_SendEvent_(&e))
		return true;
	
	// no message function (and last window in thie main level): end program
	// ...or at least end nested main level
	int n = 0;
	foreach(HuiWindow *w, HuiWindows)
		if (w->_GetMainLevel_() >= win->_GetMainLevel_())
			n ++;
	if (n == 1)
		HuiEnd();
	return false;
}

void OnGtkWindowResize(GtkWidget *widget, GtkRequisition *requisition, gpointer user_data)
{
	HuiWindow *win = (HuiWindow *)user_data;
	HuiEvent e = HuiEvent("", "hui:resize");
	win->_SendEvent_(&e);
}

gboolean OnGtkWindowMouseMove(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	HuiWindow *win = (HuiWindow*)user_data;

	//msg_write(format("  %d  %d", (int)event->x, (int)event->y));

	int mx, my, mod = 0;
	if (win->gl_widget){
		//msg_write("gl");
#if GTK_MAJOR_VERSION >= 3
		gdk_window_get_device_position(gtk_widget_get_window(win->gl_widget), event->device, &mx, &my, (GdkModifierType*)&mod);
#else
		gdk_window_get_pointer(win->gl_widget->window, &mx, &my, (GdkModifierType*)&mod);
#endif
		//mx = event->x;
		//my = event->y;
		/*win->mouse_offset_x = mx - event->x;
		win->mouse_offset_y = my - event->y;*/
		mod = event->state;
		/*mx = event->x;
		my = event->y;*/
	}else{
		// TODO GTK3
		//gdk_window_get_pointer(win->window->window, &mx, &my, (GdkModifierType*)&mod);
		irect ri = win->GetInterior();
		mx -= ri.x1;
		my -= ri.y1;
	}
	//msg_write(format("gtk  %.0f %.0f  ->  %d %d   (%d)", win->input.x, win->input.y, mx, my, gdk_event_get_time((GdkEvent*)event)));
	
	/*if (ignore_time != 0)
		if (gdk_event_get_time((GdkEvent*)event) <= ignore_time){
			//msg_error("ignore");
			gdk_event_request_motions(event); // to prevent too many signals for slow message processing
			return false;
		}*/
	win->input.dx = mx - win->input.x;
	win->input.dy = my - win->input.y;
	win->input.x = mx;
	win->input.y = my;
	win->input.lb = ((mod & GDK_BUTTON1_MASK) > 0);
	win->input.mb = ((mod & GDK_BUTTON2_MASK) > 0);
	win->input.rb = ((mod & GDK_BUTTON3_MASK) > 0);
	HuiEvent e = HuiEvent("", "hui:mouse-move");
	win->_SendEvent_(&e);

	/*if (win){
		// don't listen to "event", it lacks behind
		int mod = 0, mx, my, x_max, y_max;
		if (win->input_widget){
			msg_write("iw");
			gdk_window_get_pointer(win->input_widget->window, &mx, &my, (GdkModifierType*)&mod);
			win->input.x = mx;
			win->input.y = my;
		}else{
			msg_write("!iw");
			// TODO
			if ((win->Resizable) && (win->_cur_control_))
				gdk_window_get_pointer(win->_cur_control_->widget->window, &mx, &my, (GdkModifierType*)&mod);
			else
				gdk_window_get_pointer(win->window->window, &mx, &my, (GdkModifierType*)&mod);
			irect ri = win->GetInterior();
			x_max = ri.x2 - ri.x1;
			y_max = ri.y2 - ri.y1;
			win->input.x = mx - ri.x1;
			win->input.y = my - ri.y1;
			//msg_write(string2("%d  %d  %f - %d %d %f", ri.x1, mx, win->input.x, ri.y1, my, win->input.y));
		}

		// only allow setting when cursor is in the interior
		if ((win->input.x < 0) || (win->input.x >= x_max) || (win->input.y < 0) || (win->input.y >= y_max)){
			// outside -> only reset allowed
			win->input.lb &= ((mod & GDK_BUTTON1_MASK) > 0);
			win->input.mb &= ((mod & GDK_BUTTON2_MASK) > 0);
			win->input.rb &= ((mod & GDK_BUTTON3_MASK) > 0);
		}else{
			win->input.lb = ((mod & GDK_BUTTON1_MASK) > 0);
			win->input.mb = ((mod & GDK_BUTTON2_MASK) > 0);
			win->input.rb = ((mod & GDK_BUTTON3_MASK) > 0);
		}
		HuiEvent e = HuiCreateEvent("", "hui:mouse-move");
		win->_SendEvent_(&e);
	}*/
	gdk_event_request_motions(event); // too prevent too many signals for slow message processing
	return false;
}

gboolean OnGtkWindowMouseWheel(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	HuiWindow *win = (HuiWindow*)user_data;
	if (win){
		if (event->direction == GDK_SCROLL_UP)
			win->input.dz = 1;
		else if (event->direction == GDK_SCROLL_DOWN)
			win->input.dz = -1;
		HuiEvent e = HuiEvent("", "hui:mouse-wheel");
		win->_SendEvent_(&e);
	}
	return false;
}

gboolean OnGtkWindowKeyDown(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	process_key(event, widget, (HuiWindow*)user_data, true);
	return false;
}

gboolean OnGtkWindowKeyUp(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	process_key(event, widget, (HuiWindow*)user_data, false);
	return false;
}

gboolean OnGtkWindowExpose(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	HuiWindow *win = (HuiWindow*)user_data;
	HuiEvent e = HuiEvent("", "hui:redraw");
	win->_SendEvent_(&e);
	return false;
}

gboolean expose_event_gl(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	//msg_write(string2("expose gl %d", xpi++));
	HuiWindow *win = (HuiWindow*)user_data;
	HuiEvent e = HuiEvent("", "hui:redraw");
	win->_SendEvent_(&e);
	return true; // stop handler...
}

gboolean OnGtkWindowVisibilityNotify(GtkWidget *widget, GdkEventVisibility *event, gpointer user_data)
{
	HuiWindow *win = (HuiWindow*)user_data;
	HuiEvent e = HuiEvent("", "hui:redraw");
	win->_SendEvent_(&e);
	return false;
}

bool set_button_state(GtkWidget *widget, GdkEventButton *event)
{
	HuiWindow *win = win_from_widget(widget);
	if (win){
		if (event->type == GDK_BUTTON_PRESS){
			irect r = win->GetInterior();
			if ((win->input.x < 0) || (win->input.x >= r.x2 - r.x1) || (win->input.y < 0) || (win->input.y >= r.y2 - r.y1))
				return false;
		}
		// don't listen to "event", it lacks behind
		if (event->button == 1)
			win->input.lb = (event->type == GDK_BUTTON_PRESS);
		else if (event->button == 2)
			win->input.mb = (event->type == GDK_BUTTON_PRESS);
		else if (event->button == 3)
			win->input.rb = (event->type == GDK_BUTTON_PRESS);
	}
	return true;
}

gboolean OnGtkWindowButtonDown(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
#ifdef OS_WINDOWS
	// without this... nothing happens.... code optimization???
	//msg_write("");
#endif
	HuiWindow *win = (HuiWindow*)user_data;
	if (set_button_state(widget, event)){
		string msg = "hui:";
		if (event->button == 1)
			msg += "left";
		else if (event->button == 2)
			msg += "middle";
		else if (event->button == 3)
			msg += "right";
		if (event->type == GDK_2BUTTON_PRESS)
			msg += "-double-click";
		else
			msg += "-button-down";
		HuiEvent e = HuiEvent("", msg);
		win->_SendEvent_(&e);
	}
	return false;
}

gboolean OnGtkWindowButtonUp(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	HuiWindow *win = (HuiWindow*)user_data;
	if (set_button_state(widget, event)){
		string msg = "hui:";
		if (event->button == 1)
			msg += "left";
		else if (event->button == 2)
			msg += "middle";
		else if (event->button == 3)
			msg += "right";
		msg += "-button-up";
		HuiEvent e = HuiEvent("", msg);
		win->_SendEvent_(&e);
	}
	return false;
}

gboolean focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	// make sure the contro/alt/shift keys are unset
	HuiWindow *win = win_from_widget(widget);
	if (win){

		// reset all keys
		memset(&win->input.key, 0, sizeof(win->input.key));
		/*win->input.key[KEY_RSHIFT] = win->input.key[KEY_LSHIFT] = false;
		win->input.key[KEY_RCONTROL] = win->input.key[KEY_LCONTROL] = false;
		win->input.key[KEY_RALT] = win->input.key[KEY_LALT] = false;*/
	}
	return false;
}




//----------------------------------------------------------------------------------
// window functions


// general window

void HuiWindow::_Init_(const string &title, int x, int y, int width, int height, HuiWindow *root, bool allow_root, int mode)
{
	msg_db_r("HuiWindow()",1);

	_InitGeneric_(root, allow_root, mode);
	
	bool ControlMode = ((mode & HuiWinModeControls) > 0);
	bool NixMode = ((mode & HuiWinModeNix) > 0);

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
		gtk_widget_hide(gtk_dialog_get_action_area(GTK_DIALOG(window)));
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

	// icon
	if (HuiPropLogo.num > 0)
		gtk_window_set_icon_from_file(GTK_WINDOW(window), sys_str_f(HuiPropLogo), NULL);

	// catch signals
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(&OnGtkWindowClose), this);
	if (NixMode)
		AllowEvents("key,mouse,scroll,draw");

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
	toolbar[HuiToolbarTop].widget = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar[HuiToolbarTop].widget), true);
	toolbar[HuiToolbarBottom].widget = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar[HuiToolbarBottom].widget),true);
	toolbar[HuiToolbarLeft].widget = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar[HuiToolbarLeft].widget), true);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(toolbar[HuiToolbarLeft].widget), GTK_ORIENTATION_VERTICAL);
	toolbar[HuiToolbarRight].widget = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar[HuiToolbarRight].widget), true);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(toolbar[HuiToolbarRight].widget), GTK_ORIENTATION_VERTICAL);

	gtk_box_pack_start(GTK_BOX(vbox), toolbar[HuiToolbarTop].widget, FALSE, FALSE, 0);


	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	//gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	gtk_box_pack_start(GTK_BOX(hbox), toolbar[HuiToolbarLeft].widget, FALSE, FALSE, 0);

	plugable = NULL;
	gl_widget = NULL;
	cur_control = NULL;
	if (NixMode){
		// "drawable" (for opengl)
		gl_widget = gtk_drawing_area_new();
		gtk_widget_set_double_buffered(gl_widget, false);
		g_signal_connect(G_OBJECT(gl_widget), "draw", G_CALLBACK(&expose_event_gl), this);
		gtk_box_pack_start(GTK_BOX(hbox), gl_widget, TRUE, TRUE, 0);

		// show/realize to be done by the nix library!
		/*gtk_widget_show(gl_widget);
		gtk_widget_realize(gl_widget);
		GdkRectangle r;
		r.x = 0;
		r.y = 0;
		r.width = width;
		r.height = height;
		gdk_window_invalidate_region(gl_widget->window, gdk_region_rectangle(&r), false);
		gdk_window_process_all_updates();*/

		// prevent the toolbar from using keys...
#ifndef OS_WINDOWS
		gtk_widget_set_can_focus(gl_widget, true);
#endif
		gtk_widget_grab_focus(gl_widget);
	}else if (ControlMode){
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

	gtk_box_pack_start(GTK_BOX(hbox), toolbar[HuiToolbarRight].widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar[HuiToolbarBottom].widget, FALSE, FALSE, 0);

	// status bar
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

	input_widget = NULL;
	gtk_num_menus = 0;
	//###########################################################################################
	// never dare to ask, why there has to be a hidden edit box!
	/*if (bg_mode==HuiBGModeBlack){
		GtkWidget *edit=gtk_entry_new();
		gtk_widget_set_size_request(edit,1,1);
		gtk_fixed_put(GTK_FIXED(fixed),edit,0,0);
		gtk_widget_show(edit);
	}*/
	//###########################################################################################

	msg_db_l(1);
}

HuiWindow::~HuiWindow()
{
	msg_db_r("~CHuiWindow",1);
	_CleanUp_();

	gtk_widget_destroy(window);
	
	msg_db_l(1);
}

// should be called after creating (and filling) the window to actually show it
void HuiWindow::Show()
{
	gtk_widget_show(window);
#ifdef OS_WINDOWS
	hWnd = (HWND)gdk_win32_drawable_get_handle(window->window);
#endif

	allow_input = true;
}

string HuiWindow::Run()
{

	msg_db_r("HuiWindow.Run",1);
	Show();
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
	if (GetParent())
		gtk_dialog_run(GTK_DIALOG(window));
	else{
		bool killed = false;
		while(!killed){
			HuiDoSingleMainLoop();
			foreach(HuiClosedWindow &cw, _HuiClosedWindow_)
				if (cw.unique_id == uid)
					killed = true;
		}
	}
#endif
	//msg_write("cleanup");

	// clean up
	foreachi(HuiClosedWindow &cw, _HuiClosedWindow_, i)
		if (cw.unique_id == uid){
			last_id = cw.last_id;
			_HuiClosedWindow_.erase(i);
		}
	msg_db_l(1);
	return last_id;
}

void HuiWindow::AllowEvents(const string &msg)
{
	int mask;
	g_object_get(G_OBJECT(window), "events", &mask, NULL);
	
	// catch signals
	if (msg.find("draw") >= 0){
		//g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(&OnGtkWindowExpose), this);
		g_signal_connect(G_OBJECT(window), "visibility-notify-event", G_CALLBACK(&OnGtkWindowVisibilityNotify), this);
		mask |= GDK_VISIBILITY_NOTIFY_MASK;
	}
	if (msg.find("scroll") >= 0){
		g_signal_connect(G_OBJECT(window), "scroll-event", G_CALLBACK(&OnGtkWindowMouseWheel), this);
		mask |= GDK_SCROLL_MASK;
	}
	if (msg.find("key") >= 0){
		g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(&OnGtkWindowKeyDown), this);
		g_signal_connect(G_OBJECT(window), "key-release-event", G_CALLBACK(&OnGtkWindowKeyUp), this);
		g_signal_connect(G_OBJECT(window), "focus-in-event", G_CALLBACK(&focus_in_event), this);
		mask |= GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK;
	}
	if (msg.find("size") >= 0){
		g_signal_connect(G_OBJECT(window), "size-request", G_CALLBACK(&OnGtkWindowResize), this);
	}
	if (msg.find("mouse") >= 0){
		g_signal_connect(G_OBJECT(window), "motion-notify-event", G_CALLBACK(&OnGtkWindowMouseMove), this);
		g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(&OnGtkWindowButtonDown), this);
		g_signal_connect(G_OBJECT(window), "button-release-event", G_CALLBACK(&OnGtkWindowButtonUp), this);
		mask |= GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK; // GDK_POINTER_MOTION_HINT_MASK = "fewer motions"
	}

	g_object_set(G_OBJECT(window), "events", mask, NULL);
}

void HuiWindow::SetMenu(HuiMenu *_menu)
{
	msg_db_r("SetMenu", 1);
	// remove old menu...
	if (menu){
		// move items from <menu_bar> back to <Menu>
		for (int i=0;i<gtk_num_menus;i++){
			gtk_container_remove(GTK_CONTAINER(menubar), GTK_WIDGET(gtk_menu[i]));
			gtk_menu_shell_append(GTK_MENU_SHELL(menu->g_menu), GTK_WIDGET(gtk_menu[i]));
		}
		gtk_menu.clear();
		gtk_num_menus = 0;
	}

	
	// insert new menu
	menu = _menu;
	if (menu){
		gtk_widget_show(menubar);
		gtk_num_menus = menu->item.num;
		for (int i=0;i<menu->item.num;i++){
			// move items from <Menu> to <menu_bar>
			HuiMenuItem *it = &menu->item[i];
			gtk_menu.add(it->widget);
			gtk_widget_show(gtk_menu[i]);
			gtk_container_remove(GTK_CONTAINER(menu->g_menu), gtk_menu[i]);
			gtk_menu_shell_append(GTK_MENU_SHELL(menubar), gtk_menu[i]);
		}
	}else
		gtk_widget_hide(menubar);
	msg_db_l(1);
}

// show/hide without closing the window
void HuiWindow::Hide()
{
	gtk_widget_hide(window);
}

// set the string in the title bar
void HuiWindow::SetTitle(const string &title)
{
	gtk_window_set_title(GTK_WINDOW(window),sys_str(title));
}

// set the upper left corner of the window in screen corrdinates
void HuiWindow::SetPosition(int x, int y)
{
	gtk_window_move(GTK_WINDOW(window),x,y);
}

void HuiWindow::SetSize(int width, int height)
{
	if (parent)
		gtk_widget_set_size_request(window, width, height);
	else
		gtk_window_resize(GTK_WINDOW(window), width, height);
}

// set the current window position and size (including the frame and menu/toolbars...)
//    if maximized this will un-maximize the window!
void HuiWindow::SetOuterior(irect rect)
{
	gtk_window_unmaximize(GTK_WINDOW(window));
	gtk_window_move(GTK_WINDOW(window),rect.x1,rect.y1);
	gtk_window_resize(GTK_WINDOW(window),rect.x2-rect.x1,rect.y2-rect.y1);
}

// get the current window position and size (including the frame and menu/toolbars...)
irect HuiWindow::GetOuterior()
{
	irect r;
	gtk_window_get_position(GTK_WINDOW(window),&r.x1,&r.y1);
	gtk_window_get_size(GTK_WINDOW(window),&r.x2,&r.y2);
	r.x2+=r.x1;
	r.y2+=r.y1;
	return r;
}

// set the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <SetOuterior>
void HuiWindow::SetOuteriorDesired(irect rect)
{
	// bad hack
	bool maximized = (gdk_window_get_state(gtk_widget_get_window(window)) & GDK_WINDOW_STATE_MAXIMIZED) > 0;
	if (maximized)
		gtk_window_unmaximize(GTK_WINDOW(window));
	gtk_window_move(GTK_WINDOW(window),rect.x1,rect.y1);
	gtk_window_resize(GTK_WINDOW(window),rect.x2-rect.x1,rect.y2-rect.y1);
	if (maximized)
		gtk_window_maximize(GTK_WINDOW(window));
}

// get the window position and size it had wouldn't it be maximized (including the frame and menu/toolbars...)
//    if not maximized this behaves like <GetOuterior>
irect HuiWindow::GetOuteriorDesired()
{
	irect r;
	// bad hack
	bool maximized = (gdk_window_get_state(gtk_widget_get_window(window)) & GDK_WINDOW_STATE_MAXIMIZED) > 0;
	if (maximized){
		// very nasty hack   m(-_-)m
		r .x1 = 50;
		r .y1 = 50;
		r .x2 = r.x1 + 800;
		r .y2 = r.y1 + 600;
	}else{
		gtk_window_get_position(GTK_WINDOW(window),&r.x1,&r.y1);
		gtk_window_get_size(GTK_WINDOW(window),&r.x2,&r.y2);
		r.x2+=r.x1;
		r.y2+=r.y1;
	}
	/*if (maximized){
		gtk_window_unmaximize(GTK_WINDOW(window));
		for (int i=0;i<5;i++)
			HuiDoSingleMainLoop();
	}
	gtk_window_get_position(GTK_WINDOW(window),&r.x1,&r.y1);
	gtk_window_get_size(GTK_WINDOW(window),&r.x2,&r.y2);
	r.x2+=r.x1;
	r.y2+=r.y1;
	if (maximized){
		gtk_window_maximize(GTK_WINDOW(window));
		for (int i=0;i<20;i++)
			HuiDoSingleMainLoop();
	}*/
	return r;
}

// get the "usable" part of the window: controllers/graphics area
//   relative to the outer part!!
irect HuiWindow::GetInterior()
{
	irect r;
	r.x1 = 0;
	r.y1 = 0;
	/*int ww, hh;
	GdkWindow *www = gtk_widget_get_window(toolbar[HuiToolbarTop].widget);
	//gtk_widget_get_size_request(toolbar[HuiToolbarTop].widget, &ww, &hh);
	gdk_window_get_size(www, &ww, &hh);
	msg_write(ww);
	msg_write(hh);*/
	gtk_window_get_size(GTK_WINDOW(window), &r.x2, &r.y2);
	//msg_write(string2("getinter %d %d   %p %p", r.x2, r.y2, gl_widget, fixed));
	GtkWidget *interior = (plugable == NULL) ? gl_widget : plugable;
	if (interior){
		int w, h, x = 0, y = 0;
		gdk_window_get_position(gtk_widget_get_window(window), &x, &y);
		if (gl_widget){
			// TODO GTK3
			//gdk_drawable_get_size(gtk_widget_get_window(window), &w, &h);
			w = gdk_window_get_width(gtk_widget_get_window(gl_widget));
			h = gdk_window_get_height(gtk_widget_get_window(gl_widget));
			//msg_write(format("gl %d %d (getint)", w, h));
		}else{
			// TODO GTK3
			//gdk_window_get_size(gtk_widget_get_window(interior), &w, &h);
			w = gdk_window_get_width(gtk_widget_get_window(window));
			h = gdk_window_get_height(gtk_widget_get_window(window));
		}
		//msg_write(string2("getinter2 %d %d %d %d", x, y, w, h));
		if ((w < 0) || (h < 0) || (w > r.x2) || (h > r.y2))
			return r;
		r.x1 = x;
		r.y1 = y;
		if (plugable){
			gtk_window_get_position(GTK_WINDOW(window), &x, &y);
			r.x1 -= x;
			r.y1 -= y;
			// FIXME
			r.x1 = 0;
			r.y1 = 0;
		}
		r.x2 = r.x1 + w;
		r.y2 = r.y1 + h;
	}
	return r;
}

void HuiWindow::ShowCursor(bool show)
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

static Array<GdkEvent*> scp_event;

void ev_out(GdkEvent *e)
{
	if (e->type == 3){
		msg_write(format("event: move %.0f %.0f  (%d)", ((GdkEventMotion*)e)->x, ((GdkEventMotion*)e)->y, gdk_event_get_time(e)));
	}else
		msg_write("event: other");
}

void read_events(const string &msg, bool rem = false)
{
	foreach(GdkEvent *e, scp_event)
		gdk_event_free(e);
	scp_event.clear();
	
	int n = 0;
	while (gdk_events_pending()){
		GdkEvent *e = gdk_event_get();
		if (e){
			n ++;
			if (e->type == 3)
				gdk_event_request_motions((GdkEventMotion*)e);
			//ev_out(e);
			scp_event.add(e);
			//gdk_event_free(e);
		}else
			break;
	}
	if (n > 0){
		//msg_write(msg + ": del " + i2s(n));
		if (rem){
			gdk_event_free(scp_event.back());
			scp_event.resize(scp_event.num - 1);
		}
	}
}

void put_events()
{
	foreach(GdkEvent *e, scp_event)
		gdk_event_put(e);
}

// relative to Interior
void HuiWindow::SetCursorPos(int x, int y)
{
	if (gl_widget){
		input.x = x;
		input.y = y;
		// TODO GTK3
		XWarpPointer(hui_x_display, None, GDK_WINDOW_XID(gtk_widget_get_window(gl_widget)), 0, 0, 0, 0, x, y);
	}
#if 0
	irect ri = GetInterior();
	irect ro = GetOuterior();
	int dx = 0;
	int dy = 0;
	#ifdef OS_LINUX
		//kill_events();
		XFlush(hui_x_display);
		read_events("prae");
		float xnew = input.x, ynew = input.y;
		foreach(GdkEvent *e, scp_event){
			if (e->type == 3){
				xnew = ((GdkEventMotion*)e)->x;
				ynew = ((GdkEventMotion*)e)->y;
				ignore_time = gdk_event_get_time(e) + 20;
			}
		}
		dx = xnew - input.x;
		dy = ynew - input.y;
		//msg_write(format(" d= %d %d  (%d)", dx, dy, ignore_time));
		put_events();
		if (gl_widget){
			// TODO GTK3
			XWarpPointer(hui_x_display, None, GDK_WINDOW_XID(gtk_widget_get_window(gl_widget)), 0, 0, 0, 0, x, y);
			//XWarpPointer(hui_x_display, None, GDK_WINDOW_XWINDOW(gtk_widget_get_window(gl_widget)), 0, 0, 0, 0, x, y);
		}else
			XWarpPointer(hui_x_display, None, GDK_WINDOW_XID(gtk_widget_get_window(window)), 0, 0, 0, 0, x + ri.x1, y + ri.y1);
		XFlush(hui_x_display);
		XFlush(hui_x_display);
		/*HuiSleep(2);
		XFlush(hui_x_display);*/

		read_events("post");
		foreachb(GdkEvent *e, scp_event)
			if (e->type == 3){
				((GdkEventMotion*)e)->x = x - dx;
				((GdkEventMotion*)e)->y = y - dy;
				((GdkEventMotion*)e)->time += 100;
			}
		put_events();
	#endif
	#ifdef OS_WINDOWS
		// TODO GetInterior gives the wrong position
		//::SetCursorPos(x + ri.x1, y + ri.y1);
		::SetCursorPos(x + ri.x1, y + 20);
	#endif
	
	input.x = (float)x - dx;
	input.y = (float)y - dy;
	input.dx = 0;
	input.dy = 0;
#endif
}

void HuiWindow::SetMaximized(bool maximized)
{
	if (maximized)
		gtk_window_maximize(GTK_WINDOW(window));
	else
		gtk_window_unmaximize(GTK_WINDOW(window));
}

bool HuiWindow::IsMaximized()
{
	int state=gdk_window_get_state(gtk_widget_get_window(window));
	return ((state & GDK_WINDOW_STATE_MAXIMIZED)>0);
}

bool HuiWindow::IsMinimized()
{
	int state=gdk_window_get_state(gtk_widget_get_window(window));
	return ((state & GDK_WINDOW_STATE_ICONIFIED)>0);
}

void HuiWindow::SetFullscreen(bool fullscreen)
{
	if (fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(window));
}

void HuiWindow::EnableStatusbar(bool enabled)
{
	if (enabled)
	    gtk_widget_show(statusbar);
	else
	    gtk_widget_hide(statusbar);
	statusbar_enabled = enabled;
}

void HuiWindow::SetStatusText(const string &str)
{
	gtk_statusbar_push(GTK_STATUSBAR(statusbar),0,sys_str(str));
}


// give our window the focus....and try to focus the specified control item
void HuiWindow::Activate(const string &control_id)
{
	gtk_widget_grab_focus(window);
	gtk_window_present(GTK_WINDOW(window));
	if (control_id.num > 0)
		for (int i=0;i<control.num;i++)
			if (control_id == control[i]->id)
				gtk_widget_grab_focus(control[i]->widget);
}

bool HuiWindow::IsActive(bool include_sub_windows)
{
	bool ia=false;
	/*ghjghjghj
	// TODO!!!
	gtk_widget_grab_focus(window);
	gtk_window_present(GTK_WINDOW(window));*/
	if ((!ia)&&(include_sub_windows)){
		for (int i=0;i<sub_window.num;i++)
			if (sub_window[i]->IsActive(true))
				return true;
	}
	return ia;
}




#endif
