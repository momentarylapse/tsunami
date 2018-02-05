/*
 * ControlSlider.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLSLIDER_H_
#define CONTROLSLIDER_H_

#include "Control.h"

namespace hui
{

class ControlSlider : public Control
{
public:
	ControlSlider(const string &text, const string &id);
	virtual float getFloat();
	virtual void __setFloat(float f);
	virtual void __addString(const string &s);
	virtual void __setOption(const string &op, const string &value);
	bool vertical;
};

};

#endif /* CONTROLSLIDER_H_ */
