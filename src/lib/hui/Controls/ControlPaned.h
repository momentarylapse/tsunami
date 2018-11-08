/*
 * ControlPaned.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef CONTROLPANED_H_
#define CONTROLPANED_H_

#include "Control.h"

namespace hui
{

class ControlPaned : public Control
{
public:
	ControlPaned(const string &text, const string &id);

	void add(Control *child, int x, int y) override;
};

};

#endif /* CONTROLPANED_H_ */
