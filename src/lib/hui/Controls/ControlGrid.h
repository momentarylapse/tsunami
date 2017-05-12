/*
 * ControlGrid.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLGRID_H_
#define CONTROLGRID_H_

#include "Control.h"

namespace hui
{

class ControlGrid : public Control
{
public:
	ControlGrid(const string &text, const string &id, int num_x, int num_y, Panel *panel);
	virtual void add(Control *child, int x, int y);

	bool button_bar;
};

};

#endif /* CONTROLGRIDGTK_H_ */
