/*
 * ControlButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLBUTTON_H_
#define CONTROLBUTTON_H_

#include "Control.h"

namespace hui
{


class ControlButton : public Control
{
public:
	ControlButton(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void setImage(const string &str);
	virtual void __setOption(const string &op, const string &value);
};

};

#endif /* CONTROLBUTTON_H_ */
