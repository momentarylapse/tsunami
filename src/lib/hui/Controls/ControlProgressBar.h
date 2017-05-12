/*
 * ControlProgressBar.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLPROGRESSBAR_H_
#define CONTROLPROGRESSBAR_H_

#include "Control.h"

namespace hui
{

class ControlProgressBar : public Control
{
public:
	ControlProgressBar(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual float getFloat();
	virtual void __setFloat(float f);
};

};

#endif /* CONTROLPROGRESSBAR_H_ */
