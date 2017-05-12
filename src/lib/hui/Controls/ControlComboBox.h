/*
 * ControlComboBox.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLCOMBOBOX_H_
#define CONTROLCOMBOBOX_H_

#include "Control.h"

namespace hui
{


class ControlComboBox : public Control
{
public:
	ControlComboBox(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __addString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual void __reset();

	bool editable;
};

};

#endif /* CONTROLCOMBOBOX_H_ */
