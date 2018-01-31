/*
 * ControlMultilineEdit.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef CONTROLMULTILINEEDIT_H_
#define CONTROLMULTILINEEDIT_H_

#include "Control.h"

namespace hui
{

class ControlMultilineEdit : public Control
{
public:
	ControlMultilineEdit(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __addString(const string &str);
	virtual void setTabSize(int tab_size);
	virtual void __setOption(const string &op, const string &value);

	bool handle_keys;
};

};

#endif /* CONTROLMULTILINEEDIT_H_ */
