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
	ControlGrid(const string &text, const string &id, Panel *panel);
	void add(Control *child, int x, int y) override;

	void __set_option(const string &op, const string &value) override;

	bool button_bar;
	bool action_bar;
	bool vertical;
};

};

#endif /* CONTROLGRIDGTK_H_ */
