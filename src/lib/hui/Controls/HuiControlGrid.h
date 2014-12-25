/*
 * HuiControlGrid.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLGRID_H_
#define HUICONTROLGRID_H_

#include "HuiControl.h"


class HuiControlGrid : public HuiControl
{
public:
	HuiControlGrid(const string &text, const string &id, int num_x, int num_y, HuiPanel *panel);
	virtual void add(HuiControl *child, int x, int y);

	bool button_bar;
};

#endif /* HUICONTROLGRIDGTK_H_ */
