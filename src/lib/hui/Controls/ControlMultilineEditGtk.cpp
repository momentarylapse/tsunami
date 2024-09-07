/*
 * ControlMultilineEdit.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "ControlMultilineEdit.h"
#include "../Event.h"
#include "../language.h"
#include "../../os/msg.h"

#include <gtk/gtk.h>

namespace hui
{
#if GTK_CHECK_VERSION(3,24,0)
gboolean on_gtk_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);
void on_gtk_key_released(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);
#else
gboolean on_gtk_area_key_down(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_gtk_area_key_up(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
#endif

void on_gtk_multiline_edit_change(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);
}

ControlMultilineEdit::ControlMultilineEdit(const string &title, const string &id) :
	Control(CONTROL_MULTILINEEDIT, id)
{
	auto parts = split_title(title);
	GtkTextBuffer *tb = gtk_text_buffer_new(nullptr);
	widget = gtk_text_view_new_with_buffer(tb);
#if GTK_CHECK_VERSION(4,0,0)
	GtkWidget *scroll = gtk_scrolled_window_new();
#else
	GtkWidget *scroll = gtk_scrolled_window_new(nullptr, nullptr);
#endif
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#if GTK_CHECK_VERSION(4,0,0)
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), widget);
#else
	gtk_widget_show(scroll);
	gtk_container_add(GTK_CONTAINER(scroll), widget);
#endif

	// frame
	frame = scroll;
	if (!option_has(get_option_from_title(title), "noframe")) { //(border_width > 0){
		frame = gtk_frame_new(nullptr);
#if GTK_CHECK_VERSION(4,0,0)
		gtk_frame_set_child(GTK_FRAME(frame), scroll);
#else
		gtk_container_add(GTK_CONTAINER(frame), scroll);
#endif
	}
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	take_gtk_ownership();
	g_signal_connect(G_OBJECT(tb), "changed", G_CALLBACK(&on_gtk_multiline_edit_change), this);
	focusable = true;
	set_options(get_option_from_title(title));
}

void ControlMultilineEdit::__set_string(const string &str) {
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	const char *str2 = sys_str(str);
	gtk_text_buffer_set_text(tb, str2, strlen(str2));
}

string ControlMultilineEdit::get_string() {
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter is, ie;
	gtk_text_buffer_get_iter_at_offset(tb, &is, 0);
	gtk_text_buffer_get_iter_at_offset(tb, &ie, -1);
	return de_sys_str(gtk_text_buffer_get_text(tb, &is, &ie, false));
}

void ControlMultilineEdit::__add_string(const string& str) {
}

void ControlMultilineEdit::__reset() {
	__set_string("");
}

void ControlMultilineEdit::set_tab_size(int tab_size) {
	PangoLayout *layout = gtk_widget_create_pango_layout(widget, "W");
	int width, height;
	pango_layout_get_pixel_size(layout, &width, &height);
	PangoTabArray *ta = pango_tab_array_new(1, true);
	pango_tab_array_set_tab(ta, 0, PANGO_TAB_LEFT, width * tab_size);
	gtk_text_view_set_tabs(GTK_TEXT_VIEW(widget), ta);
}


void ControlMultilineEdit::__set_option(const string &op, const string &value) {
	if (op == "handlekeys") {
		if (value != "exclusive" and value != "")
			msg_error("handlekeys=['',exclusive]");
		user_key_handling = true;
		if (value == "exclusive")
			basic_internal_key_handling = false;
			// disable arrow keys

#if GTK_CHECK_VERSION(4,0,0)
		auto handler_key = gtk_event_controller_key_new();
		gtk_widget_add_controller(widget, handler_key);
		g_signal_connect(G_OBJECT(handler_key), "key-pressed", G_CALLBACK(&on_gtk_key_pressed), this);
		g_signal_connect(G_OBJECT(handler_key), "key-released", G_CALLBACK(&on_gtk_key_released), this);
#elif GTK_CHECK_VERSION(3,24,0)
		auto handler_key = gtk_event_controller_key_new(widget);
		g_object_weak_ref(G_OBJECT(widget), (GWeakNotify)g_object_unref, handler_key);
		g_signal_connect(G_OBJECT(handler_key), "key-pressed", G_CALLBACK(&on_gtk_key_pressed), this);
		g_signal_connect(G_OBJECT(handler_key), "key-released", G_CALLBACK(&on_gtk_key_released), this);
#else
		int mask;
		g_object_get(G_OBJECT(widget), "events", &mask, nullptr);
		mask |= GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK;
		g_object_set(G_OBJECT(widget), "events", mask, nullptr);
		g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(&on_gtk_area_key_down), this);
		g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(&on_gtk_area_key_up), this);
#endif
	} else if (op == "monospace") {
#if GTK_CHECK_VERSION(3,16,0)
		gtk_text_view_set_monospace(GTK_TEXT_VIEW(widget), true);
#else
		PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 12");
		gtk_widget_override_font(widget, font_desc);
		pango_font_description_free(font_desc);
#endif
	} else if (op == "wrap") {
		if (val_is_positive(value, true))
			gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD);
		else
			gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_NONE);
	} else if (op == "tabsize") {
		set_tab_size(value._int());
	}
}

};
