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
	GetPartStrings(title);
	widget = gtk_label_new("");
#if GTK_CHECK_VERSION(3,16,0)
	gtk_label_set_xalign(GTK_LABEL(widget), 0);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5f);
#endif
	ControlLabel::__setString(title);
	setOptions(OptionString);
}

string ControlLabel::getString()
{
	return "";
}

void ControlLabel::__setString(const string &str)
{
	GetPartStrings(str);
	string s = sys_str(PartString[0]);
	if (OptionString.find("bold") >= 0)
		s = "<b>" + s + "</b>";
	else if (OptionString.find("italic") >= 0)
		s = "<i>" + s + "</i>";
	if (OptionString.find("big") >= 0)
		s = "<big>" + s + "</big>";
	gtk_label_set_markup(GTK_LABEL(widget), s.c_str());
}

void ControlLabel::__setOption(const string &op, const string &value)
{
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
	}
}

};

#endif
