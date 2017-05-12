/*
 * ControlSpinButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLSPINBUTTON_H_
#define CONTROLSPINBUTTON_H_

#include "Control.h"

namespace hui
{

class ControlSpinButton : public Control
{
public:
	ControlSpinButton(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual float getFloat();
	virtual void __setFloat(float f);
	virtual void __setOption(const string &op, const string &value);
};

};

#endif /* CONTROLSPINBUTTON_H_ */
