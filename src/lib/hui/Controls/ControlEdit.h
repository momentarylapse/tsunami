/*
 * ControlEdit.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLEDIT_H_
#define CONTROLEDIT_H_

#include "Control.h"

namespace hui
{

class ControlEdit : public Control
{
public:
	ControlEdit(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void completionAdd(const string &text);
	virtual void completionClear();
	virtual void __setOption(const string &op, const string &value);
};

};

#endif /* CONTROLEDIT_H_ */
