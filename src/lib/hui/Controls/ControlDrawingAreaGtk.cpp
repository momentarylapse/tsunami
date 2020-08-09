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
#include "../../math/rect.h"

#include <thread>
static std::thread::id main_thread_id = std::this_thread::get_id();

#define STUPID_HACK 0

#include <GL/gl.h>

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
	da->notify("hui:draw");
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
	da->notify("hui:draw-gl");
	return false;
}

void on_gtk_gl_area_realize(GtkGLArea *area, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	gtk_gl_area_make_current(area);
	if (gtk_gl_area_get_error(area) != nullptr){
		printf("realize: gl area make current error...\n");
		return;
	}

	da->notify("hui:realize");
}

void on_gtk_gl_area_unrealize(GtkGLArea *area, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	gtk_gl_area_make_current(area);
	if (gtk_gl_area_get_error(area) != nullptr){
		printf("unrealize: gl area make current error...\n");
		return;
	}
	da->notify("hui:unrealize");
}

/*void OnGtkAreaResize(GtkWidget *widget, GtkRequisition *requisition, gpointer user_data)
{	NotifyWindowByWidget((CHuiWindow*)user_data, widget, "hui:resize", false);	}*/

template<class T>
void win_set_input(Window *win, T *event) {
	if (event->type == GDK_ENTER_NOTIFY) {
		win->input.inside = true;
	} else if (event->type == GDK_LEAVE_NOTIFY) {
		win->input.inside = false;
	}
	win->input.dx = (float)event->x - win->input.x;
	win->input.dy = (float)event->y - win->input.y;
	//msg_write(format("%.1f\t%.1f\t->\t%.1f\t%.1f\t(%.1f\t%.1f)", win->input.x, win->input.y, event->x, event->y, win->input.dx, win->input.dy));
	win->input.scroll_x = 0;
	win->input.scroll_y = 0;
	win->input.x = (float)event->x;
	win->input.y = (float)event->y;
	win->input.pressure = 1;
	int mod = event->state;
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

	Control *c = reinterpret_cast<Control*>(user_data);
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

	c->notify("hui:mouse-move", false);
	gdk_event_request_motions(event); // to prevent too many signals for slow message processing
	return false;
}

gboolean on_gtk_area_mouse_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	Control *c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);

	c->notify("hui:mouse-enter", false);
	return false;
}

gboolean on_gtk_area_mouse_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	Control *c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify("hui:mouse-leave", false);
	return false;
}

gboolean on_gtk_area_button(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	Control *c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	win_set_input_more(c->panel->win, event);

	// build message
	string msg = "hui:";
	if (event->button == 1)
		msg += "left";
	else if (event->button == 2)
		msg += "middle";
	else if (event->button == 3)
		msg += "right";
	if (event->type == GDK_2BUTTON_PRESS)
		msg += "-double-click";
	else if (event->type == GDK_BUTTON_PRESS)
		msg += "-button-down";
	else
		msg += "-button-up";

	if (!gtk_widget_has_focus(widget)) {
		gtk_widget_grab_focus(widget);
		c->panel->win->input.just_focused = true;
	}
	c->notify(msg, false);
	return false;
}

gboolean on_gtk_area_focus_in(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	Control *c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify("hui:focus-in", false);
	return false;
}

gboolean on_gtk_area_mouse_wheel(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
	Control *c = reinterpret_cast<Control*>(user_data);
	if (c->panel->win) {
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
		c->notify("hui:mouse-wheel", false);
	}
	return false;
}

void _get_hui_key_id_(GdkEventKey *event, int &key, int &key_code) {
	// convert hardware keycode into GDK keyvalue
	GdkKeymapKey kmk;
	kmk.keycode = event->hardware_keycode;
	kmk.group = event->group;
	kmk.level = 0;
	auto *map = gdk_keymap_get_for_display(gdk_display_get_default());
	int keyvalue = gdk_keymap_lookup_key(map, &kmk);
	// TODO GTK3
	//int keyvalue = event->keyval;
	//msg_write(keyvalue);

	// convert GDK keyvalue into HUI key id
	key = -1;
	for (int i=0;i<NUM_KEYS;i++)
		//if ((HuiKeyID[i] == keyvalue)or(HuiKeyID2[i] == keyvalue))
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

	c->notify(down ? "hui:key-down" : "hui:key-up", false);

	// stop further gtk key handling
	return c->grab_focus;
}



gboolean on_gtk_area_key_down(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	return area_process_key(event, (Control*)user_data, true);
}

gboolean on_gtk_area_key_up(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	return area_process_key(event, (Control*)user_data, false);
}

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
	g_signal_connect(G_OBJECT(da), "key-press-event", G_CALLBACK(&on_gtk_area_key_down), this);
	g_signal_connect(G_OBJECT(da), "key-release-event", G_CALLBACK(&on_gtk_area_key_up), this);
	//g_signal_connect(G_OBJECT(da), "size-request", G_CALLBACK(&OnGtkAreaResize), this);
	g_signal_connect(G_OBJECT(da), "motion-notify-event", G_CALLBACK(&on_gtk_area_mouse_move), this);
	g_signal_connect(G_OBJECT(da), "enter-notify-event", G_CALLBACK(&on_gtk_area_mouse_enter), this);
	g_signal_connect(G_OBJECT(da), "leave-notify-event", G_CALLBACK(&on_gtk_area_mouse_leave), this);
	g_signal_connect(G_OBJECT(da), "button-press-event", G_CALLBACK(&on_gtk_area_button), this);
	g_signal_connect(G_OBJECT(da), "button-release-event", G_CALLBACK(&on_gtk_area_button), this);
	g_signal_connect(G_OBJECT(da), "scroll-event", G_CALLBACK(&on_gtk_area_mouse_wheel), this);
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
	Control *c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.scroll_x = scale;
	c->panel->win->input.scroll_y = scale;
	c->notify("hui:gesture-zoom", false);
}

void on_gtk_gesture_zoom_begin(GtkGestureZoom *controller, GdkEventSequence *sequence, gpointer user_data) {
	Control *c = reinterpret_cast<Control*>(user_data);
	c->notify("hui:gesture-zoom-begin", false);
}

void on_gtk_gesture_zoom_end(GtkGestureZoom *controller, GdkEventSequence *sequence, gpointer user_data) {
	Control *c = reinterpret_cast<Control*>(user_data);
	c->notify("hui:gesture-zoom-end", false);
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
