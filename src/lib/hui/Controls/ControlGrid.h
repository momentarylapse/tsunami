/*
 * ControlGrid.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLGRID_H_
#define CONTROLGRID_H_

#include "Control.h"

namespace hui {

class ControlGrid : public Control {
public:
	ControlGrid(const string &text, const string &id, Panel *panel);
	void add_child(shared<Control> child, int x, int y) override;
	void remove_child(Control *child) override;

	void __set_option(const string &op, const string &value) override;

	bool is_button_bar;
	bool is_action_bar;
	bool is_vertical;
	bool is_box;
};

};

#endif /* CONTROLGRIDGTK_H_ */
