/*
 * HuiControlLabel.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */


#include "HuiControlLabel.h"

#ifdef HUI_API_GTK

HuiControlLabel::HuiControlLabel(const string &title, const string &id) :
	HuiControl(HUI_KIND_LABEL, id)
{
	GetPartStrings(id, title);
	widget = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5f);
	HuiControlLabel::__setString(title);
	setOptions(OptionString);
}

string HuiControlLabel::getString()
{
	return "";
}

void HuiControlLabel::__setString(const string &str)
{
	GetPartStrings(id, str);
	string s = sys_str(PartString[0]);
	if (OptionString.find("bold") >= 0)
		s = "<b>" + s + "</b>";
	else if (OptionString.find("italic") >= 0)
		s = "<i>" + s + "</i>";
	if (OptionString.find("big") >= 0)
		s = "<big>" + s + "</big>";
	gtk_label_set_markup(GTK_LABEL(widget), s.c_str());
}

void HuiControlLabel::__setOption(const string &op, const string &value)
{
	if (op == "wrap")
		gtk_label_set_line_wrap(GTK_LABEL(widget),true);
	else if (op == "center")
		gtk_misc_set_alignment(GTK_MISC(widget), 0.5f, 0.5f);
	else if (op == "right")
		gtk_misc_set_alignment(GTK_MISC(widget), 1, 0.5f);
	else if (op == "angle")
		gtk_label_set_angle(GTK_LABEL(widget), value._float());
}

#endif
