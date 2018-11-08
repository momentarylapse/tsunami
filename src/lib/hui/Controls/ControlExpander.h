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

	void expand(int row, bool expand) override;
	void expand_all(bool expand) override;
	bool is_expanded(int row) override;

	void add(Control *child, int x, int y) override;
};

};

#endif /* CONTROLEXPANDER_H_ */
