/*
 * ToolItemButton.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef TOOLITEMBUTTON_H_
#define TOOLITEMBUTTON_H_

#include "Control.h"

namespace hui
{

class ToolItemButton : public Control
{
public:
	ToolItemButton(const string &title, const string &image, const string &id);
};

}

#endif /* TOOLITEMBUTTON_H_ */
