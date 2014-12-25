/*
 * HuiControlComboBox.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLCOMBOBOX_H_
#define HUICONTROLCOMBOBOX_H_

#include "HuiControl.h"


class HuiControlComboBox : public HuiControl
{
public:
	HuiControlComboBox(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __addString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual void __reset();
};

#endif /* HUICONTROLCOMBOBOX_H_ */
