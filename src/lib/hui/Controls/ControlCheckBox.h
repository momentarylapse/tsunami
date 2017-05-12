/*
 * ControlCheckBox.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLCHECKBOX_H_
#define CONTROLCHECKBOX_H_

#include "Control.h"

namespace hui
{


class ControlCheckBox : public Control
{
public:
	ControlCheckBox(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __check(bool checked);
	virtual bool isChecked();
};

};

#endif /* CONTROLCHECKBOX_H_ */
