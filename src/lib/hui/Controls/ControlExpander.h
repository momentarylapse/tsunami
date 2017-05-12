/*
 * ControlExpander.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef CONTROLEXPANDER_H_
#define CONTROLEXPANDER_H_

#include "Control.h"

namespace hui
{

class ControlExpander : public Control
{
public:
	ControlExpander(const string &text, const string &id);

	virtual void expand(int row, bool expand);
	virtual void expandAll(bool expand);
	virtual bool isExpanded(int row);

	virtual void add(Control *child, int x, int y);
};

};

#endif /* CONTROLEXPANDER_H_ */
