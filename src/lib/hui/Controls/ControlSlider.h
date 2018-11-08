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
	float get_float() override;
	void __set_float(float f) override;
	void __add_string(const string &s) override;
	void __set_option(const string &op, const string &value) override;
	bool vertical;
};

};

#endif /* CONTROLSLIDER_H_ */
