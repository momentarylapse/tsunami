/*
 * ControlScroller.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef CONTROLSCROLLER_H_
#define CONTROLSCROLLER_H_

#include "Control.h"

namespace hui
{

class ControlScroller : public Control
{
public:
	ControlScroller(const string &text, const string &id);

	void add(Control *child, int x, int y) override;
	void __set_option(const string &op, const string &value) override;

	GtkWidget *viewport;
};

};

#endif /* CONTROLSCROLLER_H_ */
