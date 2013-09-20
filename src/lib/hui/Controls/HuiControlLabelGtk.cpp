/*
 * HuiControlLabel.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */


#include "HuiControlLabel.h"

#ifdef HUI_API_GTK

HuiControlLabel::HuiControlLabel(const string &title, const string &id) :
	HuiControl(HuiKindText, id)
{
	GetPartStrings(id, title);
	widget = gtk_label_new("");
	if (OptionString.find("wrap") >= 0)
		gtk_label_set_line_wrap(GTK_LABEL(widget),true);
	if (OptionString.find("center") >= 0)
		gtk_misc_set_alignment(GTK_MISC(widget), 0.5f, 0.5f);
	else if (OptionString.find("right") >= 0)
		gtk_misc_set_alignment(GTK_MISC(widget), 1, 0.5f);
	else
		gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5f);
	HuiControlLabel::__SetString(title);
	SetOptions(OptionString);
}

string HuiControlLabel::GetString()
{
	return "";
}

HuiControlLabel::~HuiControlLabel()
{
}

void HuiControlLabel::__SetString(const string &str)
{
	GetPartStrings(id, str);
	string s = sys_str(PartString[0]);
	if (OptionString.find("bold") >= 0)
		s = "<b>" + s + "</b>";
	else if (OptionString.find("italic") >= 0)
		s = "<i>" + s + "</i>";
	gtk_label_set_markup(GTK_LABEL(widget), s.c_str());
}

#endif
