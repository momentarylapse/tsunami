/*
 * HuiControlGrid.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlGrid.h"
#include "../hui.h"

#ifdef HUI_API_GTK

HuiControlGrid::HuiControlGrid(const string &title, const string &id, int num_x, int num_y, HuiWindow *win) :
	HuiControl(HuiKindControlTable, id)
{
	GetPartStrings(id, title);
	widget = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(widget), win->border_width);
	gtk_grid_set_column_spacing(GTK_GRID(widget), win->border_width);
	SetOptions(OptionString);
	if (OptionString.find("buttonbar") >= 0){
		gtk_widget_set_margin_top(widget, 7);
		gtk_widget_set_margin_bottom(widget, 4);
	}
}

HuiControlGrid::~HuiControlGrid() {
	// TODO Auto-generated destructor stub
}

void HuiControlGrid::add(HuiControl *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	gtk_grid_attach(GTK_GRID(widget), child_widget, x, y, 1, 1);
	child->parent = this;
	children.add(child);
}

#endif
