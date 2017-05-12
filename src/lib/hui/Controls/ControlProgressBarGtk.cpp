/*
 * ControlProgressBar.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlProgressBar.h"


#ifdef HUI_API_GTK

namespace hui
{

ControlProgressBar::ControlProgressBar(const string &title, const string &id) :
	Control(CONTROL_PROGRESSBAR, id)
{
	GetPartStrings(title);
	widget = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), sys_str(PartString[0]));
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
	setOptions(OptionString);
}

string ControlProgressBar::getString()
{
	return "";
}

void ControlProgressBar::__setString(const string &str)
{
#if GTK_CHECK_VERSION(3,0,0)
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widget), true);
#endif
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), sys_str(str));
}

float ControlProgressBar::getFloat()
{
	return 0;
}

void ControlProgressBar::__setFloat(float f)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), min(max(f, 0.0f), 1.0f));
}

};

#endif
