/*
 * ControlExpanderGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "ControlExpander.h"
#include "../language.h"
#include "../Window.h"
#include "../../os/msg.h"

#include <gtk/gtk.h>

namespace hui {

//const int FRAME_INDENT = 0; //20;

void on_gtk_expander_expand(GObject* object, GParamSpec *param_spec, gpointer user_data) {
	auto expander = reinterpret_cast<ControlExpander*>(user_data);
	bool expanded = gtk_expander_get_expanded(GTK_EXPANDER(object));
	if (expanded) {
		gtk_widget_set_vexpand_set(GTK_WIDGET(object), false);
	} else {
		gtk_widget_set_vexpand(GTK_WIDGET(object), false);
	}

	// header clicked -> also change the revealer with the children
	gtk_revealer_set_reveal_child(GTK_REVEALER(expander->revealer), expanded);
}

// ok, gtk is weird here:
//  * gtk_expander: no animation, but header
//  * gtk_revealer: animation, but no header
// so we combine both

// Two modes:
//  * title empty -> no header
//    - any animation works
//  * title nonempty -> with header
//    - best with up/down (default)

ControlExpander::ControlExpander(const string &title, const string &id) :
	Control(CONTROL_EXPANDER, id)
{
	auto parts = split_title(title);

#if !GTK_CHECK_VERSION(3,10,0)
#error "GTK too old..."
#endif

	if (parts[0].num > 0) {
		// with header

		frame = gtk_grid_new();
		gtk_widget_set_margin_bottom(frame, 12);

		expander = gtk_expander_new(sys_str("<b>" + parts[0] + "</b>"));
		gtk_expander_set_use_markup(GTK_EXPANDER(expander), true);
		gtk_grid_attach(GTK_GRID(frame), expander, 0, 0, 1, 1);
		g_signal_connect(expander, "notify::expanded", G_CALLBACK(on_gtk_expander_expand), this);
		gtk_widget_set_margin_bottom(expander, 6);

		revealer = gtk_revealer_new();
		gtk_grid_attach(GTK_GRID(frame), revealer, 0, 1, 1, 1);
#if !GTK_CHECK_VERSION(4,0,0)
		gtk_widget_show(revealer);
#endif

		widget = expander;
	} else {
		// no header
		revealer = gtk_revealer_new();
		expander = nullptr;
		widget = revealer;

		//if (!gtk_expander_get_expanded(GTK_EXPANDER(widget)))
		//	gtk_widget_set_vexpand(widget, false);
	}
	gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), false);
	take_gtk_ownership();

	//set_style_for_widget(widget, id, ".expander:active{transition: 300ms linear;}");


	//if (!gtk_expander_get_expanded(GTK_EXPANDER(widget)))
	//	gtk_widget_set_vexpand(widget, false);

	set_options(get_option_from_title(title));
}

void ControlExpander::add_child(shared<Control> child, int x, int y) {
	GtkWidget *child_widget = child->get_frame();
#if GTK_CHECK_VERSION(4,0,0)
	gtk_revealer_set_child(GTK_REVEALER(revealer), child_widget);
#else
	gtk_container_add(GTK_CONTAINER(revealer), child_widget);
#endif
	control_link(this, child);
}

# if 0
void ControlExpander::add_child(Control *child, int x, int y) {
	GtkWidget *child_widget = child->get_frame();
	//gtk_widget_set_vexpand(child_widget, true);
	//gtk_widget_set_hexpand(child_widget, true);
	int ind = child->indent;
	if (ind < 0)
		ind = FRAME_INDENT;
#if GTK_CHECK_VERSION(3,12,0)
	gtk_widget_set_margin_start(child_widget, ind);
#else
	gtk_widget_set_margin_left(child_widget, ind);
#endif

#if GTK_CHECK_VERSION(4,0,0)
	gtk_revealer_set_child(GTK_REVEALER(revealer), child_widget);
#else
	gtk_container_add(GTK_CONTAINER(revealer), child_widget);
#endif

/*#if GTK_CHECK_VERSION(4,0,0)
	gtk_expander_set_child(GTK_EXPANDER(widget), child_widget);
#else
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
#endif*/
	control_link(this, child);
}
#endif

void ControlExpander::remove_child(Control *child) {
	msg_write("Expander.remove");
#if GTK_CHECK_VERSION(4,0,0)
	gtk_revealer_set_child(GTK_REVEALER(revealer), nullptr);
#else
	GtkWidget *child_widget = child->get_frame();
	gtk_container_remove(GTK_CONTAINER(revealer), child_widget);
#endif
	control_unlink(this, child);
}



void ControlExpander::expand(int row, bool expanded) {
	gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), expanded);
	if (expander)
		gtk_expander_set_expanded(GTK_EXPANDER(expander), expanded);
}

bool ControlExpander::is_expanded(int row) {
	return gtk_revealer_get_reveal_child(GTK_REVEALER(revealer));
}

void ControlExpander::__set_option(const string& op, const string& value) {
	if (op == "slide") {
		if (value == "up")
			gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
		else if (value == "down")
			gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
		else if (value == "left")
			gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
		else if (value == "right")
			gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
	} else if (op == "crossfade") {
		gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
	}
}

};
