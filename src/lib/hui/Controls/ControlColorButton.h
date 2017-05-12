/*
 * ControlColorButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLCOLORBUTTON_H_
#define CONTROLCOLORBUTTON_H_

#include "Control.h"

namespace hui
{


class ControlColorButton : public Control
{
public:
	ControlColorButton(const string &text, const string &id);

	virtual void __setColor(const color &c);
	virtual color getColor();
};

};

#endif /* CONTROLCOLORBUTTON_H_ */
