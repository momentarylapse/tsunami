/*
 * HuiControlProgressBar.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlProgressBar.h"

HuiControlProgressBar::HuiControlProgressBar(const string &title, const string &id) :
	HuiControl(HuiKindProgressBar, id)
{
	GetPartStrings(id, title);
	widget = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), sys_str(PartString[0]));
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
}

HuiControlProgressBar::~HuiControlProgressBar() {
	// TODO Auto-generated destructor stub
}

string HuiControlProgressBar::GetString()
{
}

void HuiControlProgressBar::__SetString(const string &str)
{
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widget), true);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), sys_str(str));
}

float HuiControlProgressBar::GetFloat()
{
}

void HuiControlProgressBar::__SetFloat(float f)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), min(max(f, 0), 1));
}
