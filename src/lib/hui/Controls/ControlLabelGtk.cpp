/*
 * HuiControlLabel.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */


#include "ControlLabel.h"

#ifdef HUI_API_GTK

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
	flag_bold = flag_italic = flag_big = flag_small = false;
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
	string s = sys_str(text);
	if (flag_bold)
		s = "<b>" + text + "</b>";
	if (flag_italic)
		s = "<i>" + s + "</i>";
	if (flag_underline)
		s = "<u>" + s + "</u>";
	if (flag_strikeout)
		s = "<s>" + s + "</s>";
	if (flag_big)
		s = "<big>" + s + "</big>";
	else if (flag_small)
		s = "<small>" + s + "</small>";
	gtk_label_set_markup(GTK_LABEL(widget), s.c_str());
}

void ControlLabel::__set_option(const string &op, const string &value) {
	if (op == "wrap"){
		gtk_label_set_line_wrap(GTK_LABEL(widget), true);
	}else if (op == "center"){
#if GTK_CHECK_VERSION(3,16,0)
		gtk_label_set_xalign(GTK_LABEL(widget), 0.5f);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), 0.5f, 0.5f);
#endif
	}else if (op == "right"){
#if GTK_CHECK_VERSION(3,16,0)
		gtk_label_set_xalign(GTK_LABEL(widget), 1);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), 1, 0.5f);
#endif
	}else if (op == "angle"){
		gtk_label_set_angle(GTK_LABEL(widget), value._float());
	}else if (op == "bold"){
		flag_bold = true;
	}else if (op == "italic"){
		flag_italic = true;
	}else if (op == "big"){
		flag_big = true;
	}else if (op == "small"){
		flag_small = true;
	}else if (op == "underline"){
		flag_underline = true;
	}else if (op == "strikeout"){
		flag_strikeout = true;
	}
}

};

#endif
