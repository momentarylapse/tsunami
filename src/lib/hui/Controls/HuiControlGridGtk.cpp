/*
 * HuiControlGrid.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlGrid.h"15
#include "../hui.h"

HuiControlGrid::HuiControlGrid(const string &title, const string &id, int num_x, int num_y, HuiWindow *win) :
	HuiControl(HuiKindControlTable, id)
{
	GetPartStrings(id, title);
	widget = gtk_table_new(num_y, num_x, false);
	gtk_table_set_row_spacings(GTK_TABLE(widget), win->border_width);
	gtk_table_set_col_spacings(GTK_TABLE(widget), win->border_width);
}

HuiControlGrid::~HuiControlGrid() {
	// TODO Auto-generated destructor stub
}

