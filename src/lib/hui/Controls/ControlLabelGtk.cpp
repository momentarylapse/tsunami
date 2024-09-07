/*
 * HuiControlLabel.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */


#include "ControlLabel.h"
#include <gtk/gtk.h>

namespace hui
{

ControlLabel::ControlLabel(const string &title, const string &id) :
	Control(CONTROL_LABEL, id)
{
	auto parts = split_title(title);
	widget = gtk_label_new("");
#if GTK_CHECK_VERSION(3,16,0)
	gtk_label_set_xalign(GTK_LABEL(widget), 0);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5f);
#endif
	take_gtk_ownership();
	flag_bold = flag_italic = false;
	flag_underline = flag_strikeout = false;
	set_options(get_option_from_title(title));
	ControlLabel::__set_string(title);
}

string ControlLabel::get_string() {
	return "";
}

void ControlLabel::__set_string(const string &str) {
	auto parts = split_title(str);
	text = parts[0];
	set_options(get_option_from_title(str));
	string s = text;
	if (flag_bold)
		s = "<b>" + s + "</b>";
	if (flag_italic)
		s = "<i>" + s + "</i>";
	if (flag_underline)
		s = "<u>" + s + "</u>";
	if (flag_strikeout)
		s = "<s>" + s + "</s>";
	gtk_label_set_markup(GTK_LABEL(widget), s.c_str());
}

void ControlLabel::__set_option(const string &op, const string &value) {
	if (op == "wrap") {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_label_set_wrap(GTK_LABEL(widget), true);
#else
		gtk_label_set_line_wrap(GTK_LABEL(widget), true);
#endif
	} else if (op == "center") {
#if GTK_CHECK_VERSION(3,16,0)
		gtk_label_set_xalign(GTK_LABEL(widget), 0.5f);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), 0.5f, 0.5f);
#endif
	} else if (op == "right") {
#if GTK_CHECK_VERSION(3,16,0)
		gtk_label_set_xalign(GTK_LABEL(widget), 1);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), 1, 0.5f);
#endif
	} else if (op == "angle") {
#if GTK_CHECK_VERSION(4,0,0)
		if (fabs(value._float() - 90) < 20)
			add_css_class("hui-rotate-left");
		else
			add_css_class("hui-rotate-left");
#else
		gtk_label_set_angle(GTK_LABEL(widget), value._float());
#endif
	} else if (op == "bold") {
		flag_bold = true;
	} else if (op == "italic") {
		flag_italic = true;
	} else if (op == "underline") {
		flag_underline = true;
	} else if (op == "strikeout") {
		flag_strikeout = true;
	} else if (op == "style") {
		this->add_css_class(value);
	}
}

};

