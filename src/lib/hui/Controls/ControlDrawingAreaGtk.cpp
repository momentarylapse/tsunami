/*
 * HuiControlDrawingArea.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlDrawingArea.h"
#include "../internal.h"
#include "../Window.h"
#include "../../image/color.h"
#include "../../math/rect.h"
#include "../../os/config.h"
#include "../../os/msg.h"
#include <cmath>
#include <thread>

#include <gtk/gtk.h>

static std::thread::id main_thread_id = std::this_thread::get_id();


/*#if !GTK_CHECK_VERSION(3,24,0)
#error gtk >= 3.24 required
#endif*/

#define STUPID_HACK 0

#ifdef HUI_API_GTK

namespace hui
{

GdkGLContext *gtk_gl_context = nullptr;

static base::set<ControlDrawingArea*> _recently_deleted_areas;

color color_from_gdk(const GdkRGBA &gcol);


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

base::map<string,color> get_style_colors(Panel *p, const string &id) {
	base::map<string,color> colors;
	auto c = p->_get_control_(id);
	if (!c)
		return colors;
	GtkStyleContext *sc = gtk_widget_get_style_context(c->widget);
	Array<string> names = {"base_color", "text_color", "fg_color", "bg_color", "selected_fg_color", "selected_bg_color", "insensitive_fg_color", "insensitive_bg_color", "borders", "unfocused_borders", "warning_wolor", "error_color", "success_color"};
	for (auto &name: names) {
		GdkRGBA cc;
		if (gtk_style_context_lookup_color(sc, ("theme_" + name).c_str(), &cc))
			colors.set(name, color_from_gdk(cc));
		else if (gtk_style_context_lookup_color(sc, name.c_str(), &cc))
			colors.set(name, color_from_gdk(cc));
	}
	return colors;
}

#pragma GCC diagnostic pop


/*---------------------------------------------------------------------------------*
 * drawing                                                                         */


int get_screen_scale(GtkWidget *w) {
#if GTK_CHECK_VERSION(4,0,0)
	return gtk_widget_get_scale_factor(w);
#else
	return gdk_monitor_get_scale_factor(gdk_display_get_monitor_at_window(gdk_display_get_default(), gtk_widget_get_parent_window(w)));
#endif
}


#if GTK_CHECK_VERSION(4,0,0)

void on_gtk_area_draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);
	//std::lock_guard<std::mutex> lock(da->mutex);
	da->cur_cairo = cr;
	//msg_write("draw " + reinterpret_cast<ControlDrawingArea*>(user_data)->id);
#if STUPID_HACK
	da->redraw_area.clear();
#endif
	int scale = get_screen_scale(GTK_WIDGET(da->widget));
	da->panel->win->input.row_target = scale;
	da->notify(EventID::DRAW);
	//msg_write("/draw " + da->id);
}

#else

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

#endif



/*---------------------------------------------------------------------------------*
 * OpenGL                                                                          */


gboolean on_gtk_gl_area_render(GtkGLArea *area, GdkGLContext *context, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	int scale = get_screen_scale(GTK_WIDGET(area));
#if GTK_CHECK_VERSION(4,12,0)
	graphene_rect_t r;
	if (gtk_widget_compute_bounds(GTK_WIDGET(area), GTK_WIDGET(area), &r)) {
		da->panel->win->input.row = (int) (r.size.height * (float) scale);
		da->panel->win->input.column = (int) (r.size.width * (float) scale);
	}
#else
	GtkAllocation a;
	gtk_widget_get_allocation(GTK_WIDGET(area), &a);
	da->panel->win->input.row = a.height * scale;
	da->panel->win->input.column = a.width * scale;
#endif
	da->panel->win->input.row_target = scale;

	gtk_gl_context = context;
	da->notify(EventID::DRAW_GL);
	return false;
}


extern int allow_signal_level; // -> hui_window_control.cpp

void on_gtk_gl_area_realize(GtkGLArea *area, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	gtk_gl_area_make_current(area);
	if (auto error = gtk_gl_area_get_error(area)) {
		msg_error(format("hui realize: gtk_gl_area_make_current() error: %s", error->message));
		return;
	}
	da->notify(EventID::REALIZE);
}

void on_gtk_gl_area_unrealize(GtkGLArea *area, gpointer user_data) {
	auto *da = reinterpret_cast<ControlDrawingArea*>(user_data);

	gtk_gl_area_make_current(area);
	if (auto error = gtk_gl_area_get_error(area)) {
		msg_error(format("hui unrealize: gtk_gl_area_make_current() error: %s", error->message));
		return;
	}
	da->notify(EventID::UNREALIZE);
}


/*---------------------------------------------------------------------------------*
 * helper                                                                          */


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

void win_correct_by_modifier(Window *win, GdkModifierType mod) {
	if ((mod & GDK_CONTROL_MASK) and !win->get_key(KEY_CONTROL))
		win->input.key[KEY_LCONTROL] = true;
	else if (!(mod & GDK_CONTROL_MASK))
		win->input.key[KEY_LCONTROL] = win->input.key[KEY_RCONTROL] = false;

	if ((mod & GDK_SHIFT_MASK) and !win->get_key(KEY_SHIFT))
		win->input.key[KEY_LSHIFT] = true;
	else if (!(mod & GDK_SHIFT_MASK))
		win->input.key[KEY_LSHIFT] = win->input.key[KEY_RSHIFT] = false;

#if !GTK_CHECK_VERSION(4,0,0)
	const int GDK_ALT_MASK = GDK_META_MASK;
#endif
	if ((mod & GDK_ALT_MASK) and !win->get_key(KEY_ALT))
		win->input.key[KEY_LALT] = true;
	else if (!(mod & GDK_ALT_MASK))
		win->input.key[KEY_LALT] = win->input.key[KEY_RALT] = false;

	// mouse buttons
	win->input.lb = (mod & GDK_BUTTON1_MASK);
	win->input.mb = (mod & GDK_BUTTON2_MASK);
	win->input.rb = (mod & GDK_BUTTON3_MASK);
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

int decode_gtk_keycode(int keycode) {
	int basic_keyval = -1;
	guint *rkeys = nullptr;
	int nkeys = -1;
#if GTK_CHECK_VERSION(4,0,0)
	if (gdk_display_map_keycode(gdk_display_get_default(), keycode, nullptr, &rkeys, &nkeys)) {
#else
	auto map = gdk_keymap_get_for_display(gdk_display_get_default());
	if (gdk_keymap_get_entries_for_keycode(map, keycode, nullptr, &rkeys, &nkeys)) {
#endif
		if (nkeys > 0)
			basic_keyval = rkeys[0];
		g_free(rkeys);
	}
	if (basic_keyval < 0)
		return -1;

	for (int i=0; i<NUM_KEYS; i++)
		if (HuiKeyID[i] == basic_keyval)
			return i;
	return -1;
}

int _get_hui_key_id_2(guint keyval, guint keycode, int mod) {

	Event::_text = utf32_to_utf8({(int)gdk_keyval_to_unicode(keyval)});

	// yes, gtk3 has a direct event->keyval, but we need the "basic" key (without modifiers)

	//msg_write(format("%d  %d", (int)event->keyval, (int)event->hardware_keycode));
	// convert GDK keyvalue into HUI key id

	int key_code = decode_gtk_keycode(keycode);
	if (key_code < 0)
		key_code = decode_gtk_keyval(keyval);
	if (key_code < 0)
		return -1;


	// key code?
	if ((mod & GDK_CONTROL_MASK) > 0)
		key_code += KEY_CONTROL;
	if ((mod & GDK_SHIFT_MASK) > 0)
		key_code += KEY_SHIFT;
	if ((mod & GDK_META_MASK) > 0)
		key_code += KEY_META;
	if ((mod & GDK_HYPER_MASK) > 0)
		key_code += KEY_HYPER;
#if GTK_CHECK_VERSION(4,0,0)
	if ((mod & GDK_ALT_MASK) > 0)
		key_code += KEY_ALT;
#else
	if (((mod & GDK_MOD1_MASK) > 0) /*or ((event->state & GDK_MOD2_MASK) > 0) or ((event->state & GDK_MOD5_MASK) > 0)*/)
		key_code += KEY_ALT;
#endif
	return key_code;
}

#if !GTK_CHECK_VERSION(4,0,0)
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
#endif



/*---------------------------------------------------------------------------------*
 * mouse motion                                                                    */


#if GTK_CHECK_VERSION(3,24,0)
// gtk 3/4

void on_gtk_gesture_motion(GtkEventControllerMotion *controller, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_mouse_pos(c->panel->win, (float)x, (float)y);
	win_set_modifier_keys(c->panel->win);
#if GTK_CHECK_VERSION(4,0,0)
	auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(controller));
	win_correct_by_modifier(c->panel->win, mod);
	c->panel->win->input.row = gdk_device_get_source(gtk_event_controller_get_current_event_device(GTK_EVENT_CONTROLLER(controller)));
#endif
	c->notify(EventID::MOUSE_MOVE, false);
}

#else

// gtk < 3.24
gboolean on_gtk_area_mouse_move(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {

	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	win_set_input_more(c->panel->win, event);

	// gtk hinting system doesn't work?
	// always use the real (current) cursor
	int x, y, mod = 0;
	#if GTK_MAJOR_VERSION >= 3
		gdk_window_get_device_position(gtk_widget_get_window(c->widget), event->device, &x, &y, (GdkModifierType*)&mod);
		// x,y seem to be window relative (not widget)...
	#else
		gdk_window_get_pointer(c->widget->window, &x, &y, (GdkModifierType*)&mod);
	#endif

	c->panel->win->input.x = (float)event->x;
	c->panel->win->input.y = (float)event->y;

	//if (!c->panel->win->input.lb)
		c->notify(EventID::MOUSE_MOVE, false);
	//gdk_event_request_motions(event); // to prevent too many signals for slow message processing
	return false;
}
#endif




/*---------------------------------------------------------------------------------*
 * enter/leave                                                                     */


#if GTK_CHECK_VERSION(3,24,0)
// gtk3/4

void on_gtk_motion_enter(GtkEventControllerMotion *controller, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_mouse_pos(c->panel->win, (float)x, (float)y);
#if GTK_CHECK_VERSION(4,0,0)
	auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(controller));
	win_correct_by_modifier(c->panel->win, mod);
#endif
	c->notify(EventID::MOUSE_ENTER, false);
}

void on_gtk_motion_leave(GtkEventControllerMotion *controller, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->notify(EventID::MOUSE_LEAVE, false);
}

#else

gboolean on_gtk_area_mouse_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify(EventID::MOUSE_ENTER, false);
	return false;
}

gboolean on_gtk_area_mouse_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify(EventID::MOUSE_LEAVE, false);
	return false;
}
#endif




/*---------------------------------------------------------------------------------*
 * mouse click                                                                     */


#if GTK_CHECK_VERSION(4,0,0)

void on_gtk_gesture_click_pressed(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(gesture));
	win_correct_by_modifier(c->panel->win, mod);
	win_set_mouse_pos(c->panel->win, (float)x, (float)y);

	int but = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
	if (but == GDK_BUTTON_PRIMARY) {
		c->notify(EventID::LEFT_BUTTON_DOWN, false);
		if (n_press >= 2)
			c->notify(EventID::LEFT_DOUBLE_CLICK, false);
	} else if (but == GDK_BUTTON_MIDDLE) {
		c->notify(EventID::MIDDLE_BUTTON_DOWN, false);
	} else if (but == GDK_BUTTON_SECONDARY) {
		c->notify(EventID::RIGHT_BUTTON_DOWN, false);
	}

	if (c->focusable)
		gtk_widget_grab_focus(c->widget);
}

void on_gtk_gesture_click_released(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_mouse_pos(c->panel->win, (float)x, (float)y);
	auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(gesture));
	win_correct_by_modifier(c->panel->win, mod);

	int but = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
	if (but == GDK_BUTTON_PRIMARY) {
		c->notify(EventID::LEFT_BUTTON_UP, false);
	} else if (but == GDK_BUTTON_MIDDLE) {
		c->notify(EventID::MIDDLE_BUTTON_UP, false);
	} else if (but == GDK_BUTTON_SECONDARY) {
		c->notify(EventID::RIGHT_BUTTON_UP, false);
	}
}

#else
// gtk 3

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

#endif



/*---------------------------------------------------------------------------------*
 * focus                                                                           */


#if GTK_CHECK_VERSION(4,0,0)

void on_gtk_focus_enter(GtkEventControllerFocus *controller, gpointer user_data) {
	/*auto c = reinterpret_cast<Control*>(user_data);
	c->notify(EventID::MOUSE_ENTER, false);*/
}

#else

// UNUSED
gboolean on_gtk_area_focus_in(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	win_set_input(c->panel->win, event);
	c->notify(EventID::FOCUS_IN, false);
	return false;
}

#endif



/*---------------------------------------------------------------------------------*
 * scroll                                                                          */


//#if GTK_CHECK_VERSION(3,24,0)
#if GTK_CHECK_VERSION(4,0,0)

void on_gtk_gesture_scroll(GtkEventControllerScroll *controller, double dx, double dy, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);

#if GTK_CHECK_VERSION(4,8,0)
	const auto unit = gtk_event_controller_scroll_get_unit(controller);
	if (unit == GDK_SCROLL_UNIT_SURFACE) { // px
		// we're mostly expecting "clicks" ... 1 click ~ 25px
		dx *= 0.04;
		dy *= 0.04;
	}
#endif
	const float factor = config.get_float("hui.scroll-factor", 1.0f);
	c->panel->win->input.scroll_x = (float)dx * factor;
	c->panel->win->input.scroll_y = (float)dy * factor;

	auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(controller));
	win_correct_by_modifier(c->panel->win, mod);

	c->notify(EventID::MOUSE_WHEEL, false);
}

#else

// use this also for gtk >= 3.24, because it allows to update modifiers

gboolean on_gtk_area_mouse_wheel(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	const float factor = config.get_float("hui.scroll-factor", 1.0f);

	if (c->panel->win) {
		win_correct_by_modifier(c->panel->win, (GdkModifierType)event->state);
		if (event->direction == GDK_SCROLL_UP) {
			c->panel->win->input.scroll_y = factor;
		} else if (event->direction == GDK_SCROLL_DOWN) {
			c->panel->win->input.scroll_y = - factor;
		} else if (event->direction == GDK_SCROLL_LEFT) {
			c->panel->win->input.scroll_x = factor;
		} else if (event->direction == GDK_SCROLL_RIGHT) {
			c->panel->win->input.scroll_x = - factor;
		} else if (event->direction == GDK_SCROLL_SMOOTH) {
			c->panel->win->input.scroll_x = (float)event->delta_x * factor;
			c->panel->win->input.scroll_y = (float)event->delta_y * factor;
		}
		c->notify(EventID::MOUSE_WHEEL, false);
	}
	return false;
}

#endif



/*---------------------------------------------------------------------------------*
 * keys                                                                            */




#if GTK_CHECK_VERSION(3,24,0)
// gtk3/4

gboolean on_gtk_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);

	int key_code = _get_hui_key_id_2(keyval, keycode, state);
	if (key_code < 0)
		return false;

	int key = (key_code & 0xff);
	c->panel->win->input.key[key] = true;
	c->panel->win->input.key_code = key_code;
#if GTK_CHECK_VERSION(4,0,0)
	/*auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(controller));
	win_correct_by_modifier(c->panel->win, mod);*/
#endif
	c->notify(EventID::KEY_DOWN, false);

	bool is_special = false;
	if (key_code == KEY_CONTROL + KEY_C or key_code == KEY_CONTROL + KEY_V)
		is_special = true;
	if (key_code == KEY_CONTROL + KEY_Z or key_code == KEY_CONTROL + KEY_Y)
		is_special = true;
	bool is_extra_special = false;
	if (key_code == KEY_LEFT or key_code == KEY_RIGHT or key_code == KEY_UP or key_code == KEY_DOWN)
		is_extra_special = true;
	if (key_code == KEY_F7)
		is_extra_special = true;

#if GTK_CHECK_VERSION(4,0,0)
	if (c->allow_global_key_shortcuts and (!c->basic_internal_key_handling or is_special or is_extra_special))
		c->panel->win->_try_send_by_key_code_(key_code);
#else
	if (c->allow_global_key_shortcuts)
		c->panel->win->_try_send_by_key_code_(key_code);
#endif


	// stop further gtk key handling (also multi line edit...)
	return !c->basic_internal_key_handling or is_special;
}

void on_gtk_key_released(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);

	int key_code = _get_hui_key_id_2(keyval, keycode, state);
	if (key_code < 0)
		return;

	int key = (key_code & 0xff);
	c->panel->win->input.key[key] = false;
	c->panel->win->input.key_code = key_code;
#if GTK_CHECK_VERSION(4,0,0)
	/*auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(controller));
	win_correct_by_modifier(c->panel->win, mod);*/
#endif
	c->notify(EventID::KEY_UP, false);
}

#else
// gtk < 3.24

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
	return !c->internal_key_handling;
}

gboolean on_gtk_area_key_down(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	return area_process_key(event, (Control*)user_data, true);
}

gboolean on_gtk_area_key_up(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	return area_process_key(event, (Control*)user_data, false);
}
#endif









// UNUSED
/*void on_gtk_gesture_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	double x0, y0;
	gtk_gesture_drag_get_start_point(gesture, &x0, &y0);
	win_set_mouse_pos(c->panel->win, float(x0 + offset_x), float(y0 + offset_y));

#if GTK_CHECK_VERSION(4,0,0)
	auto mod = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(gesture));
	win_correct_by_modifier(c->panel->win, mod);
#endif
	win_set_modifier_keys(c->panel->win);
	c->panel->win->input.lb = true;
	c->panel->win->input.mb = false;
	c->panel->win->input.rb = false;
	c->notify(EventID::MOUSE_MOVE, false);
}*/








ControlDrawingArea::ControlDrawingArea(const string &title, const string &id) :
	Control(CONTROL_DRAWINGAREA, id)
{
	basic_internal_key_handling = false;
#if STUPID_HACK
	delay_timer = new Timer;
#endif
	// FIXME: this needs to be supplied as title... fromSource() won't work...
	is_opengl = option_has(get_option_from_title(title), "opengl");
	GtkWidget *da;
	if (is_opengl) {
		da = gtk_gl_area_new();
		auto vv = option_value(get_option_from_title(title), "opengl").explode(".");
		if (vv.num == 2)
			gtk_gl_area_set_required_version(GTK_GL_AREA(da), vv[0]._int(), vv[1]._int());
#if GTK_CHECK_VERSION(4,12,0)
		gtk_gl_area_set_allowed_apis(GTK_GL_AREA(da), GDK_GL_API_GL);
#endif
		gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(da), true);
		gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(da), true);
		gtk_gl_area_attach_buffers(GTK_GL_AREA(da));
		g_signal_connect(G_OBJECT(da), "realize", G_CALLBACK(on_gtk_gl_area_realize), this);
		g_signal_connect(G_OBJECT(da), "unrealize", G_CALLBACK(on_gtk_gl_area_unrealize), this);
		g_signal_connect(G_OBJECT(da), "render", G_CALLBACK(on_gtk_gl_area_render), this);
	} else {
		da = gtk_drawing_area_new();
#if GTK_CHECK_VERSION(4,0,0)
		gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(da), on_gtk_area_draw, this, nullptr);
#else
		g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(on_gtk_area_draw), this);
#endif
	}
	widget = da;



	// mouse motion
#if GTK_CHECK_VERSION(4,0,0)
	auto handler_motion = gtk_event_controller_motion_new();
	__gtk_add_controller(handler_motion);
	g_signal_connect(G_OBJECT(handler_motion), "motion", G_CALLBACK(&on_gtk_gesture_motion), this);
	// somehow getting ignored?
	g_signal_connect(G_OBJECT(handler_motion), "enter", G_CALLBACK(&on_gtk_motion_enter), this);
	g_signal_connect(G_OBJECT(handler_motion), "leave", G_CALLBACK(&on_gtk_motion_leave), this);
#elif GTK_CHECK_VERSION(3,24,0)
	auto handler_motion = gtk_event_controller_motion_new(da);
	g_signal_connect(G_OBJECT(handler_motion), "motion", G_CALLBACK(&on_gtk_gesture_motion), this);
	// somehow getting ignored?
	g_signal_connect(G_OBJECT(handler_motion), "enter", G_CALLBACK(&on_gtk_motion_enter), this);
	g_signal_connect(G_OBJECT(handler_motion), "leave", G_CALLBACK(&on_gtk_motion_leave), this);
	g_object_weak_ref(G_OBJECT(da), (GWeakNotify)g_object_unref, handler_motion);
#else
	g_signal_connect(G_OBJECT(da), "motion-notify-event", G_CALLBACK(&on_gtk_area_mouse_move), this);
	g_signal_connect(G_OBJECT(da), "enter-notify-event", G_CALLBACK(&on_gtk_area_mouse_enter), this);
	g_signal_connect(G_OBJECT(da), "leave-notify-event", G_CALLBACK(&on_gtk_area_mouse_leave), this);
#endif


	// mouse click
#if GTK_CHECK_VERSION(4,0,0)
	auto gesture_click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture_click), 0);
	__gtk_add_controller(GTK_EVENT_CONTROLLER(gesture_click));
	g_signal_connect(G_OBJECT(gesture_click), "pressed", G_CALLBACK(&on_gtk_gesture_click_pressed), this);
	g_signal_connect(G_OBJECT(gesture_click), "released", G_CALLBACK(&on_gtk_gesture_click_released), this);
#else
	g_signal_connect(G_OBJECT(da), "button-press-event", G_CALLBACK(&on_gtk_area_button), this);
	g_signal_connect(G_OBJECT(da), "button-release-event", G_CALLBACK(&on_gtk_area_button), this);
#endif


	// scroll
#if GTK_CHECK_VERSION(4,0,0)
	auto handler_scroll = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
	__gtk_add_controller(handler_scroll);
	g_signal_connect(G_OBJECT(handler_scroll), "scroll", G_CALLBACK(&on_gtk_gesture_scroll), this);
/*#elif GTK_CHECK_VERSION(3,24,0)
	auto handler_scroll = gtk_event_controller_scroll_new(da, GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
	g_object_weak_ref(G_OBJECT(da), (GWeakNotify)g_object_unref, handler_scroll);
	g_signal_connect(G_OBJECT(handler_scroll), "scroll", G_CALLBACK(&on_gtk_gesture_scroll), this);*/
#else
	g_signal_connect(G_OBJECT(da), "scroll-event", G_CALLBACK(&on_gtk_area_mouse_wheel), this);
#endif


	// keys
#if GTK_CHECK_VERSION(4,0,0)
	auto handler_key = gtk_event_controller_key_new();
	__gtk_add_controller(handler_key);
	g_signal_connect(G_OBJECT(handler_key), "key-pressed", G_CALLBACK(&on_gtk_key_pressed), this);
	g_signal_connect(G_OBJECT(handler_key), "key-released", G_CALLBACK(&on_gtk_key_released), this);
#elif GTK_CHECK_VERSION(3,24,0)
	auto handler_key = gtk_event_controller_key_new(da);
	g_object_weak_ref(G_OBJECT(da), (GWeakNotify)g_object_unref, handler_key);
	g_signal_connect(G_OBJECT(handler_key), "key-pressed", G_CALLBACK(&on_gtk_key_pressed), this);
	g_signal_connect(G_OBJECT(handler_key), "key-released", G_CALLBACK(&on_gtk_key_released), this);
#else
	g_signal_connect(G_OBJECT(da), "key-press-event", G_CALLBACK(&on_gtk_area_key_down), this);
	g_signal_connect(G_OBJECT(da), "key-release-event", G_CALLBACK(&on_gtk_area_key_up), this);
#endif


	// focus
#if GTK_CHECK_VERSION(4,0,0)
#if 0
	auto handler_focus = gtk_event_controller_focus_new();
	__gtk_add_controller(handler_focus);
	g_signal_connect(G_OBJECT(handler_focus), "enter", G_CALLBACK(&on_gtk_focus_enter), this);
#endif
#else
//	g_signal_connect(G_OBJECT(da), "focus-in-event", G_CALLBACK(&on_gtk_area_focus_in), this);
#endif



#if !GTK_CHECK_VERSION(4,0,0)
	//g_signal_connect(G_OBJECT(da), "size-request", G_CALLBACK(&OnGtkAreaResize), this);
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
#endif



///	auto handler_drag = gtk_gesture_drag_new(da);
///	g_signal_connect(G_OBJECT(handler_drag), "drag-update", G_CALLBACK(&on_gtk_gesture_drag_update), this);

	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	set_options(get_option_from_title(title));
	take_gtk_ownership();

	cur_cairo = nullptr;
}

ControlDrawingArea::~ControlDrawingArea() {
	_recently_deleted_areas.add(this);

#if STUPID_HACK
	delete delay_timer;
#endif

	// clean-up list later
	hui::run_later(10, [this]{ _recently_deleted_areas.erase(this); });
}

void ControlDrawingArea::make_current() {
	if (is_opengl)
		gtk_gl_area_make_current(GTK_GL_AREA(widget));
}

#if GTK_CHECK_VERSION(4,0,0)
void ControlDrawingArea::disable_event_handlers_rec() {
	for (auto c: __controllers)
		gtk_widget_remove_controller(widget, c);
	Control::disable_event_handlers_rec();
}

void ControlDrawingArea::__gtk_add_controller(GtkEventController* controller) {
	gtk_widget_add_controller(widget, controller);
	__controllers.add(controller);
}
#endif

void on_gtk_gesture_zoom(GtkGestureZoom *gesture, gdouble scale, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.scroll_x = scale;
	c->panel->win->input.scroll_y = scale;
	c->panel->win->input.row = gdk_device_get_source(gtk_gesture_get_device(GTK_GESTURE(gesture)));
	c->notify(EventID::GESTURE_ZOOM, false);
}

void on_gtk_gesture_zoom_begin(GtkGestureZoom *gesture, GdkEventSequence *sequence, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.row = gdk_device_get_source(gtk_gesture_get_device(GTK_GESTURE(gesture)));
	c->notify(EventID::GESTURE_ZOOM_BEGIN, false);
}

void on_gtk_gesture_zoom_end(GtkGestureZoom *gesture, GdkEventSequence *sequence, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.row = gdk_device_get_source(gtk_gesture_get_device(GTK_GESTURE(gesture)));
	c->notify(EventID::GESTURE_ZOOM_END, false);
}

void on_gtk_gesture_drag_begin(GtkGestureDrag* gesture, gdouble start_x, gdouble start_y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.scroll_x = start_x;
	c->panel->win->input.scroll_y = start_y;
	// TODO find better drag notification system
	c->panel->win->input.row = gdk_device_get_source(gtk_gesture_get_device(GTK_GESTURE(gesture)));
	c->notify(EventID::GESTURE_DRAG_BEGIN, false);
}

void on_gtk_gesture_drag_end(GtkGestureDrag* gesture, gdouble start_x, gdouble start_y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.scroll_x = start_x;
	c->panel->win->input.scroll_y = start_y;
	c->panel->win->input.row = gdk_device_get_source(gtk_gesture_get_device(GTK_GESTURE(gesture)));
	c->notify(EventID::GESTURE_DRAG_END, false);
}

void on_gtk_gesture_drag_update(GtkGestureDrag* gesture, gdouble x, gdouble y, gpointer user_data) {
	auto c = reinterpret_cast<Control*>(user_data);
	c->panel->win->input.scroll_x = x;
	c->panel->win->input.scroll_y = y;
	c->panel->win->input.row = gdk_device_get_source(gtk_gesture_get_device(GTK_GESTURE(gesture)));
	c->notify(EventID::GESTURE_DRAG, false);
}

void ControlDrawingArea::__set_option(const string &op, const string &value) {
	if (op == "makecurrentgl") {
		make_current();
	} else if (op == "gesture") {
		if (value == "zoom") {
#if GTK_CHECK_VERSION(4,0,0)
			auto gesture_zoom = gtk_gesture_zoom_new();
			__gtk_add_controller(GTK_EVENT_CONTROLLER(gesture_zoom));
#else
			auto gesture_zoom = gtk_gesture_zoom_new(widget);
#endif
			gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture_zoom), GTK_PHASE_TARGET);
			g_signal_connect(G_OBJECT(gesture_zoom), "scale-changed", G_CALLBACK(&on_gtk_gesture_zoom), this);
			g_signal_connect(G_OBJECT(gesture_zoom), "begin", G_CALLBACK(&on_gtk_gesture_zoom_begin), this);
			g_signal_connect(G_OBJECT(gesture_zoom), "end", G_CALLBACK(&on_gtk_gesture_zoom_end), this);
		} else if (value == "drag") {
#if GTK_CHECK_VERSION(4,0,0)
			auto gesture_drag = gtk_gesture_drag_new();
			__gtk_add_controller(GTK_EVENT_CONTROLLER(gesture_drag));
		//	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture_drag), GTK_PHASE_TARGET);
			g_signal_connect(G_OBJECT(gesture_drag), "drag-update", G_CALLBACK(&on_gtk_gesture_drag_update), this);
			g_signal_connect(G_OBJECT(gesture_drag), "drag-begin", G_CALLBACK(&on_gtk_gesture_drag_begin), this);
			g_signal_connect(G_OBJECT(gesture_drag), "drag-end", G_CALLBACK(&on_gtk_gesture_drag_end), this);
#endif
		}
	} else if (op == "noeventcompression") {
#if !GTK_CHECK_VERSION(4,0,0)
		hui::run_later(0.01f, [this] {
			gdk_window_set_event_compression(GDK_WINDOW(gtk_widget_get_window(widget)), false);
		});
#endif
	}
}

/*static bool __drawing_area_queue_redraw(void *p) {
	gtk_widget_queue_draw(GTK_WIDGET(p));
	return false;
}*/

void ControlDrawingArea::redraw()
{
	// non
	if (std::this_thread::get_id() != main_thread_id){
		//printf("readraw from other thread...redirect\n");
		hui::run_later(0, [this]{ redraw(); });
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
		hui::run_later(0, [this,r]{ redraw_partial(r); });
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

#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_queue_draw(widget);
#else
	gtk_widget_queue_draw_area(widget, (int)r.x1, (int)r.y1, (int)r.width(), (int)r.height());
#endif
#endif
}

};

#endif
