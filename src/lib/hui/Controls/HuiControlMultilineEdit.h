/*
 * HuiControlMultilineEdit.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLMULTILINEEDIT_H_
#define HUICONTROLMULTILINEEDIT_H_

#include "HuiControl.h"


class HuiControlMultilineEdit : public HuiControl
{
public:
	HuiControlMultilineEdit(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __addString(const string &str);
	virtual void setTabSize(int tab_size);

	bool handle_keys;
};

#endif /* HUICONTROLMULTILINEEDIT_H_ */
