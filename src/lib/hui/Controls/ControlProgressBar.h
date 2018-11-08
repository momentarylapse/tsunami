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
	string get_string() override;
	void __set_string(const string &str) override;
	float get_float() override;
	void __set_float(float f) override;
};

};

#endif /* CONTROLPROGRESSBAR_H_ */
