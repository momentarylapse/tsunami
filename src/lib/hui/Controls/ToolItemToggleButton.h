/*
 * ToolItemToggleButton.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef TOOLITEMTOGGLEBUTTON_H_
#define TOOLITEMTOGGLEBUTTON_H_

#include "Control.h"

namespace hui
{

class ToolItemToggleButton : public Control
{
public:
	ToolItemToggleButton(const string &title, const string &image, const string &id);

	void __check(bool checked) override;
	bool is_checked() override;
};

}

#endif /* TOOLITEMTOGGLEBUTTON_H_ */
