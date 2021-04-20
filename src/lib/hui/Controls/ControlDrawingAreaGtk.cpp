/*
 * HuiControlDrawingArea.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlDrawingArea.h"
#include "../hui.h"
#include <math.h>
#include "../internal.h"
#include "../Event.h"
#include "../../math/rect.h"

#include <thread>
static std::thread::id main_thread_id = std::this_thread::get_id();


#if !GTK_CHECK_VERSION(3,24,0)
#error gtk >= 3.24 required
#endif

#define STUPID_HACK 0

#ifdef HUI_API_GTK

namespace hui
{

int GtkAreaMouseSet = -1;
int GtkAreaMouseSetX, GtkAreaMouseSetY;

static ControlDrawingArea *NixGlArea = nullptr;
GdkGLContext *gtk_gl_context = nullptr;

static Set<ControlDrawingArea*> _recently_deleted_areas;



gboolean on_gtk_area_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	//std::lock_guard<std::mutex> lock(da->mutex);

	da->cur_cairo = cr;
	//msg_write("draw " + reinterpret_cast<ControlDrawingArea*>(user_data)->id);
#if STUPID_HACK
	da->redraw_area.clear();
#endif
	da->notify(EventID::DRAW);
	//msg_write("/draw " + da->id);
	return false;
}

int get_screen_scale(GtkWidget *w) {
	return gdk_monitor_get_scale_factor(gdk_display_get_monitor_at_window(gdk_display_get_default(), gtk_widget_get_parent_window(w)));
}

gboolean on_gtk_gl_area_render(GtkGLArea *area, GdkGLContext *context, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	int scale = get_screen_scale(GTK_WIDGET(area));
	GtkAllocation a;
	gtk_widget_get_allocation(GTK_WIDGET(area), &a);
	da->panel->win->input.row = a.height * scale;
	da->panel->win->input.column = a.width * scale;
	da->panel->win->input.row_target = scale;


	gtk_gl_context = context;
	da->notify(EventID::DRAW_GL);
	return false;
}

void on_gtk_gl_area_realize(GtkGLArea *area, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	gtk_gl_area_make_current(area);
	if (gtk_gl_area_get_error(area) != nullptr){
		printf("realize: gl area make current error...\n");
		return;
	}

	da->notify(EventID::REALIZE);
}

void on_gtk_gl_area_unrealize(GtkGLArea *area, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	gtk_gl_area_make_current(area);
	if (gtk_gl_area_get_error(area) != nullptr){
		printf("unrealize: gl area make current error...\n");
		return;
	}
	da->notify(EventID::UNREALIZE);
}

/*void OnGtkAreaResize(GtkWidget *widget, GtkRequisition *requisition, gpointer user_data)
{	NotifyWindowByWidget((CHuiWindow*)user_data, widget, "hui:resize", false);	}*/

void win_set_mouse_pos(Window *win, float x, float y) {
	win->input.dx = x - win->input.x;
	win->input.dy = y - win->input.y;
	win->input.x = x;
	win->input.y = y;
}

void win_set_modifier_keys(Window *win) {
	win->input.key_code = 0;
	if (win->get_key(KEY_CONTROL)) //win->mod & GDK_CONTROL_MASK)
		win->input.key_code += KEY_CONTROL;
	if (win->get_key(KEY_SHIFT)) //mod & GDK_SHIFT_MASK)
		win->input.key_code += KEY_SHIFT;
}

template<class T>
void win_set_input(Window *win, T *event) {
	if (event->type == GDK_ENTER_NOTIFY) {
		win->input.inside = true;
	} else if (event->type == GDK_LEAVE_NOTIFY) {
		win->input.inside = false;
	}
	win_set_mouse_pos(win, (float)event->x, (float)event->y);
	//msg_write(format("%.1f\t%.1f\t->\t%.1f\t%.1f\t(%.1f\t%.1f)", win->input.x, win->input.y, event->x, event->y, win->input.dx, win->input.dy));
	win->input.scroll_x = 0;
	win->input.scroll_y = 0;
	win->input.pressure = 1;
	int mod = event->state;
	//printf("mod: %x    %x %x %x\n", mod, GDK_BUTTON1_MASK, GDK_BUTTON2_MASK, GDK_BUTTON3_MASK);
	// hmmm, seems to be off sometimes... better set it by hand, later
	win->input.lb = ((mod & GDK_BUTTON1_MASK) > 0);
	win->input.mb = ((mod & GDK_BUTTON2_MASK) > 0);
	win->input.rb = ((mod & GDK_BUTTON3_MASK) > 0);
	if (win->input.lb or win->input.mb or win->input.rb) {
		win->input.inside_smart = true;
	} else {
		win->input.inside_smart = win->input.inside;
	}
	win->input.just_focused = false;
}

template<class T>
void win_set_input_more(Window *win, T *event) {
	auto dev = gdk_event_get_source_device((GdkEvent*)event);
	int n = gdk_device_get_n_axes(dev);

	for (int i=0; i<n; i++) {
		auto u = gdk_device_get_axis_use(dev, i);
		if (u == GDK_AXIS_PRESSURE)
			win->input.pressure = event->axes[i];
	}
}

gboolean on_gtk_area_mouse_move(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
	// ignore if SetCursorPosition() was used...
	if (GtkAreaMouseSet >= 0) {
		if ((fabs(event->x - GtkAreaMouseSetX) > 2.0f) or (fabs(event->y - GtkAreaMouseSetY) > 2.0f)) {
			GtkAreaMouseSet --;
			//msg_write(format("ignore fail %.0f\t%0.f", event->x, event->y));
			return false;
		}
		//msg_write(format("ignore %.0f\t%0.f", event->x, event->y));
		GtkAreaMouseSet = -1;
	}

	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	win_set_input_more(c->panel->win, event);

	// gtk hinting system doesn't work?
	// always use the real (current) cursor
/*	int x, y, mod = 0;
	#if GTK_MAJOR_VERSION >= 3
		gdk_window_get_device_position(gtk_widget_get_window(c->widget), event->device, &x, &y, (GdkModifierType*)&mod);
	#else
		gdk_window_get_pointer(c->widget->window, &x, &y, (GdkModifierType*)&mod);
	#endif
	c->win->input.x = x;
	c->win->input.y = y;*/

	if (!c->panel->win->input.lb)
		c->notify(EventID::MOUSE_MOVE, false);
//	gdk_event_request_motions(event); // to prevent too many signals for slow message processing
	return false;
}


//[[deprecated]]
gboolean on_gtk_area_mouse_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify(EventID::MOUSE_ENTER, false);
	return false;
}

//[[deprecated]]
gboolean on_gtk_area_mouse_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify(EventID::MOUSE_LEAVE, false);
	return false;
}

gboolean on_gtk_area_button(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	win_set_input_more(c->panel->win, event);

	string msg;
	auto win = c->panel->win;
	if (event->type == GDK_BUTTON_PRESS) {
		if (event->button == 1) {
			msg = EventID::LEFT_BUTTON_DOWN;
			win->input.lb = true;
		} else if (event->button == 2) {
			msg = EventID::MIDDLE_BUTTON_DOWN;
			win->input.mb = true;
		} else if (event->button == 3) {
			msg = EventID::RIGHT_BUTTON_DOWN;
			win->input.rb = true;
		}
	} else if (event->type == GDK_BUTTON_RELEASE) {
		if (event->button == 1) {
			msg = EventID::LEFT_BUTTON_UP;
			win->input.lb = false;
		} else if (event->button == 2) {
			msg = EventID::MIDDLE_BUTTON_UP;
			win->input.mb = false;
		} else if (event->button == 3) {
			msg = EventID::RIGHT_BUTTON_UP;
			win->input.rb = false;
		}
	} else if (event->type == GDK_2BUTTON_PRESS) {
		if (event->button == 1) {
			msg = EventID::LEFT_DOUBLE_CLICK;
		} else if (event->button == 2) {
			msg = EventID::MIDDLE_DOUBLE_CLICK;
		} else if (event->button == 3) {
			msg = EventID::RIGHT_DOUBLE_CLICK;
		}
	}

	if (!gtk_widget_has_focus(widget)) {
		gtk_widget_grab_focus(widget);
		c->panel->win->input.just_focused = true;
	}
	c->notify(msg, false);
	return false;
}

gboolean on_gtk_area_focus_in(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify(EventID::FOCUS_IN, false);
	return false;
}

gboolean on_gtk_area_mouse_wheel(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	if (c->panel->win) {
		win_set_modifier_keys(c->panel->win);
		if (event->direction == GDK_SCROLL_UP) {
			c->panel->win->input.scroll_y = 1;
		} else if (event->direction == GDK_SCROLL_DOWN) {
			c->panel->win->input.scroll_y = -1;
		} else if (event->direction == GDK_SCROLL_LEFT) {
			c->panel->win->input.scroll_x = -1;
		} else if (event->direction == GDK_SCROLL_RIGHT) {
			c->panel->win->input.scroll_x = -1;
		} else if (event->direction == GDK_SCROLL_SMOOTH) {
			c->panel->win->input.scroll_x = (float)event->delta_x;
			c->panel->win->input.scroll_y = (float)event->delta_y;
		}
		c->notify(EventID::MOUSE_WHEEL, false);
	}
	return false;
}

int decode_gtk_keyval(int keyval) {
	for (int i=0; i<NUM_KEYS; i++)
		//if ((HuiKeyID[i] == keyvalue)or(HuiKeyID2[i] == keyvalue))
		if (HuiKeyID[i] == keyval)
			return i;
	for (int i=0; i<26; i++)
		if (keyval == 'A' + i)
			return KEY_A + i;
	return -1;
}

void _get_hui_key_id_2(int keyval, int mod, int &key, int &key_code) {


	Event::_text = utf32_to_utf8({(int)gdk_keyval_to_unicode(keyval)});

	// yes, gtk3 has a direct event->keyval, but we need the "basic" key (without modifiers)

	//msg_write(format("%d  %d", (int)event->keyval, (int)event->hardware_keycode));
	// convert GDK keyvalue into HUI key id
	key = decode_gtk_keyval(keyval);
	key_code = key;
	if (key < 0)
		return;


	// key code?
	if ((mod & GDK_CONTROL_MASK) > 0)
		key_code += KEY_CONTROL;
	if ((mod & GDK_SHIFT_MASK) > 0)
		key_code += KEY_SHIFT;
	if (((mod & GDK_MOD1_MASK) > 0) /*or ((event->state & GDK_MOD2_MASK) > 0) or ((event->state & GDK_MOD5_MASK) > 0)*/)
		key_code += KEY_ALT;
}

void _get_hui_key_id_(GdkEventKey *event, int &key, int &key_code) {

	// convert hardware keycode into GDK keyvalue
	GdkKeymapKey kmk;
	kmk.keycode = event->hardware_keycode;
	kmk.group = 0;//event->group;
	kmk.level = 0;
	auto *map = gdk_keymap_get_for_display(gdk_display_get_default());
	int keyvalue = gdk_keymap_lookup_key(map, &kmk);

	Event::_text = utf32_to_utf8({(int)gdk_keyval_to_unicode(event->keyval)});

	// yes, gtk3 has a direct event->keyval, but we need the "basic" key (without modifiers)

	//msg_write(format("%d  %d", (int)event->keyval, (int)event->hardware_keycode));
	// convert GDK keyvalue into HUI key id
	key = decode_gtk_keyval(keyvalue);
	key_code = key;
	if (key < 0)
		return;


	// key code?
	if ((event->state & GDK_CONTROL_MASK) > 0)
		key_code += KEY_CONTROL;
	if ((event->state & GDK_SHIFT_MASK) > 0)
		key_code += KEY_SHIFT;
	if (((event->state & GDK_MOD1_MASK) > 0) /*or ((event->state & GDK_MOD2_MASK) > 0) or ((event->state & GDK_MOD5_MASK) > 0)*/)
		key_code += KEY_ALT;
}

bool area_process_key(GdkEventKey *event, Control *c, bool down) {
	int key, key_code;
	_get_hui_key_id_(event, key, key_code);
	if (key < 0)
		return false;

	//c->win->input.key_code = key;
	c->panel->win->input.key[key] = down;

	if (down) {
		c->panel->win->input.key_code = key_code;
	}

	c->notify(down ? EventID::KEY_DOWN : EventID::KEY_UP, false);

	// stop further gtk key handling
	return c->grab_focus;
}



gboolean on_gtk_area_key_down(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	return area_process_key(event, (Control*)user_data, true);
}

gboolean on_gtk_area_key_up(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	return area_process_key(event, (Control*)user_data, false);
}

void on_gtk_gesture_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	double x0, y0;
	gtk_gesture_drag_get_start_point(gesture, &x0, &y0);
	win_set_mouse_pos(c->panel->win, float(x0 + offset_x), float(y0 + offset_y));
	win_set_modifier_keys(c->panel->win);
	c->panel->win->input.lb = true;
	c->panel->win->input.mb = false;
	c->panel->win->input.rb = false;
	c->notify(EventID::MOUSE_MOVE, false);
}

void on_gtk_gesture_motion(GtkEventControllerMotion *controller, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	static int nn = 0;
	win_set_mouse_pos(c->panel->win, (float)x, (float)y);
	win_set_modifier_keys(c->panel->win);
#if GTK_CHECK_VERSION(4,0,0)
	auto mod = gtk_event_controller_get_current_event_state(controller);
	c->panel->win->input.lb = (mod & GDK_BUTTON1_MASK);
	c->panel->win->input.mb = (mod & GDK_BUTTON2_MASK);
	c->panel->win->input.rb = (mod & GDK_BUTTON3_MASK);
#endif
	c->notify(EventID::MOUSE_MOVE, false);
}

// ignored by gtk???
void on_gtk_motion_enter(GtkEventControllerMotion *controller, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	msg_write("ENTER NEW");
	win_set_mouse_pos(c->panel->win, (float)x, (float)y);
	c->notify(EventID::MOUSE_ENTER, false);
}

void on_gtk_motion_leave(GtkEventControllerMotion *controller, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	msg_write("LEAVE");
	c->notify(EventID::MOUSE_LEAVE, false);
}

// gtk 3.24
void on_gtk_gesture_scroll(GtkEventControllerScroll *controller, double dx, double dy, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.scroll_x = (float)dx;
	c->panel->win->input.scroll_y = (float)dy;
	win_set_modifier_keys(c->panel->win);
	c->notify(EventID::MOUSE_WHEEL, false);
}

gboolean on_gtk_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);

	int key, key_code;
	_get_hui_key_id_2(keyval, state, key, key_code);
	if (key < 0)
		return false;

	c->panel->win->input.key[key] = true;
	c->panel->win->input.key_code = key_code;
	c->notify(EventID::KEY_DOWN, false);

	// stop further gtk key handling
	return c->grab_focus;
}

void on_gtk_key_released(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);

	int key, key_code;
	_get_hui_key_id_2(keyval, state, key, key_code);
	if (key < 0)
		return;

	c->panel->win->input.key[key] = false;
	c->panel->win->input.key_code = key_code;
	c->notify(EventID::KEY_UP, false);
}

#if GTK_CHECK_VERSION(4,0,0)
void on_gtk_gesture_click_pressed(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_mouse_pos(c->panel->win, (float)x, (float)y);
	c->panel->win->input.lb = true;
	c->panel->win->input.mb = false;
	c->panel->win->input.rb = false;
	c->notify(EventID::LEFT_BUTTON_DOWN, false);
}
#endif

ControlDrawingArea::ControlDrawingArea(const string &title, const string &id) :
	Control(CONTROL_DRAWINGAREA, id)
{
#if STUPID_HACK
	delay_timer = new Timer;
#endif
	// FIXME: this needs to be supplied as title... fromSource() won't work...
	is_opengl = option_has(get_option_from_title(title), "opengl");
	GtkWidget *da;
	if (is_opengl) {
		NixGlArea = this;
		da = gtk_gl_area_new();
		gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(da), true);
		gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(da), true);
		gtk_gl_area_attach_buffers(GTK_GL_AREA(da));
		g_signal_connect(G_OBJECT(da), "realize", G_CALLBACK(on_gtk_gl_area_realize), this);
		g_signal_connect(G_OBJECT(da), "unrealize", G_CALLBACK(on_gtk_gl_area_unrealize), this);
		g_signal_connect(G_OBJECT(da), "render", G_CALLBACK(on_gtk_gl_area_render), this);
	} else {
		da = gtk_drawing_area_new();
		g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(on_gtk_area_draw), this);
	}
///?	g_signal_connect(G_OBJECT(da), "key-press-event", G_CALLBACK(&on_gtk_area_key_down), this);
///?	g_signal_connect(G_OBJECT(da), "key-release-event", G_CALLBACK(&on_gtk_area_key_up), this);
	//g_signal_connect(G_OBJECT(da), "size-request", G_CALLBACK(&OnGtkAreaResize), this);
////	g_signal_connect(G_OBJECT(da), "motion-notify-event", G_CALLBACK(&on_gtk_area_mouse_move), this);
	g_signal_connect(G_OBJECT(da), "enter-notify-event", G_CALLBACK(&on_gtk_area_mouse_enter), this);
	g_signal_connect(G_OBJECT(da), "leave-notify-event", G_CALLBACK(&on_gtk_area_mouse_leave), this);
	g_signal_connect(G_OBJECT(da), "button-press-event", G_CALLBACK(&on_gtk_area_button), this);
	g_signal_connect(G_OBJECT(da), "button-release-event", G_CALLBACK(&on_gtk_area_button), this);
///	g_signal_connect(G_OBJECT(da), "scroll-event", G_CALLBACK(&on_gtk_area_mouse_wheel), this);
//	g_signal_connect(G_OBJECT(da), "focus-in-event", G_CALLBACK(&on_gtk_area_focus_in), this);
	//int mask;
	//g_object_get(G_OBJECT(da), "events", &mask, NULL);
	gtk_widget_add_events(da, GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(da, GDK_POINTER_MOTION_MASK);// | GDK_FOCUS_CHANGE_MASK);// | GDK_POINTER_MOTION_HINT_MASK); // GDK_POINTER_MOTION_HINT_MASK = "fewer motions"
	gtk_widget_add_events(da, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(da, GDK_VISIBILITY_NOTIFY_MASK | GDK_SCROLL_MASK);
	gtk_widget_add_events(da, GDK_SMOOTH_SCROLL_MASK | GDK_TOUCHPAD_GESTURE_MASK);
	//mask = GDK_ALL_EVENTS_MASK;
//	g_object_set(G_OBJECT(da), "events", mask, NULL);

	auto handler_key = gtk_event_controller_key_new(da);
	g_signal_connect(G_OBJECT(handler_key), "key-pressed", G_CALLBACK(&on_gtk_key_pressed), this);
	g_signal_connect(G_OBJECT(handler_key), "key-released", G_CALLBACK(&on_gtk_key_released), this);
	g_object_weak_ref(G_OBJECT(da), (GWeakNotify)g_object_unref, handler_key);

#if GTK_CHECK_VERSION(4,0,0)
	auto gesture_click = gtk_gesture_click_new(da);
	g_signal_connect(G_OBJECT(gesture_click), "pressed", G_CALLBACK(&on_gtk_gesture_click_pressed), this);
	g_object_weak_ref(G_OBJECT(da), (GWeakNotify)g_object_unref, gesture_click);
#endif

	auto handler_motion = gtk_event_controller_motion_new(da);
	g_signal_connect(G_OBJECT(handler_motion), "motion", G_CALLBACK(&on_gtk_gesture_motion), this);
	// somehow getting ignored?
	g_signal_connect(G_OBJECT(handler_motion), "enter", G_CALLBACK(&on_gtk_motion_enter), this);
	g_signal_connect(G_OBJECT(handler_motion), "leave", G_CALLBACK(&on_gtk_motion_leave), this);
	g_object_weak_ref(G_OBJECT(da), (GWeakNotify)g_object_unref, handler_motion);

	auto handler_scroll = gtk_event_controller_scroll_new(da, GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
	g_signal_connect(G_OBJECT(handler_scroll), "scroll", G_CALLBACK(&on_gtk_gesture_scroll), this);
	g_object_weak_ref(G_OBJECT(da), (GWeakNotify)g_object_unref, handler_scroll);

///	auto handler_drag = gtk_gesture_drag_new(da);
///	g_signal_connect(G_OBJECT(handler_drag), "drag-update", G_CALLBACK(&on_gtk_gesture_drag_update), this);

	widget = da;
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	set_options(get_option_from_title(title));

	cur_cairo = nullptr;
}

ControlDrawingArea::~ControlDrawingArea() {
	_recently_deleted_areas.add(this);

#if STUPID_HACK
	delete delay_timer;
#endif

	// clean-up list later
	hui::RunLater(10, [=]{ _recently_deleted_areas.erase(this); });
}

void ControlDrawingArea::make_current() {
	if (is_opengl)
		gtk_gl_area_make_current(GTK_GL_AREA(widget));
}

void on_gtk_gesture_zoom(GtkGestureZoom *controller, gdouble scale, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.scroll_x = scale;
	c->panel->win->input.scroll_y = scale;
	c->notify(EventID::GESTURE_ZOOM, false);
}

void on_gtk_gesture_zoom_begin(GtkGestureZoom *controller, GdkEventSequence *sequence, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->notify(EventID::GESTURE_ZOOM_BEGIN, false);
}

void on_gtk_gesture_zoom_end(GtkGestureZoom *controller, GdkEventSequence *sequence, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->notify(EventID::GESTURE_ZOOM_END, false);
}

void ControlDrawingArea::__set_option(const string &op, const string &value) {
	if (op == "makecurrentgl") {
		make_current();
	} else if (op == "gesture") {
		if (value == "zoom") {
			auto gesture_zoom = gtk_gesture_zoom_new(widget);
			gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER (gesture_zoom), GTK_PHASE_TARGET);
			g_signal_connect(G_OBJECT(gesture_zoom), "scale-changed", G_CALLBACK(&on_gtk_gesture_zoom), this);
			g_signal_connect(G_OBJECT(gesture_zoom), "begin", G_CALLBACK(&on_gtk_gesture_zoom_begin), this);
			g_signal_connect(G_OBJECT(gesture_zoom), "end", G_CALLBACK(&on_gtk_gesture_zoom_end), this);
		}
	} else if (op == "noeventcompression") {
		hui::RunLater(0.01f, [=] {
			gdk_window_set_event_compression(GDK_WINDOW(gtk_widget_get_window(widget)), false);
		});
	}
}

static bool __drawing_area_queue_redraw(void *p) {
	gtk_widget_queue_draw(GTK_WIDGET(p));
	return false;
}

void ControlDrawingArea::redraw()
{
	// non
	if (std::this_thread::get_id() != main_thread_id){
		//printf("readraw from other thread...redirect\n");
		hui::RunLater(0, [=]{ redraw(); });
		return;
	}


#if STUPID_HACK

	//std::lock_guard<std::mutex> lock(mutex);

	if (is_opengl){
		gtk_widget_queue_draw(widget);
		return;
	}

	//msg_write("redraw " + id);
	rect r = rect(0,0,1000000,1000000);
	if (delay_timer->peek() < 0.2f){
		for (rect &rr: redraw_area)
			if (rr.covers(r)){
				//msg_write("    IGNORE " + f2s(delay_timer->peek(), 3));
				return;
			}
	}else{
		delay_timer->reset();
		redraw_area.clear();
	}
	if (!widget)
		return;
	//printf("                    DRAW\n");
#if 1
	gtk_widget_queue_draw(widget);
#else
	g_idle_add((GSourceFunc)__drawing_area_queue_redraw,(void*)widget);
#endif
	redraw_area.add(r);
#else
	if (_recently_deleted_areas.contains(this)){
		//msg_error("saved by me!!!!");
		return;
	}

	gtk_widget_queue_draw(widget);
#endif
}

void ControlDrawingArea::redraw_partial(const rect &r)
{
	if (std::this_thread::get_id() != main_thread_id){
		//printf("readraw from other thread...redirect\n");
		hui::RunLater(0, [this,r]{ redraw_partial(r); });
		return;
	}
	//std::lock_guard<std::mutex> lock(mutex);

#if STUPID_HACK
	if (is_opengl){
		gtk_widget_queue_draw_area(widget, r.x1, r.y1, r.width(), r.height());
		return;
	}

	if (delay_timer->peek() < 0.2f){
		for (rect &rr: redraw_area)
			if (rr.covers(r)){
				//msg_write("    IGNORE " + f2s(delay_timer->peek(), 3));
				return;
			}
	}else{
		delay_timer->reset();
		redraw_area.clear();
	}
	if (!widget)
		return;
	gtk_widget_queue_draw_area(widget, r.x1, r.y1, r.width(), r.height());
	redraw_area.add(r);
#else
	if (_recently_deleted_areas.contains(this)){
		//msg_error("saved by me!!!!");
		return;
	}

	gtk_widget_queue_draw_area(widget, (int)r.x1, (int)r.y1, (int)r.width(), (int)r.height());
#endif
}

};

#endif
