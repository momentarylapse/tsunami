/*
 * HuiControlEdit.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLEDIT_H_
#define HUICONTROLEDIT_H_

#include "HuiControl.h"


class HuiControlEdit : public HuiControl
{
public:
	HuiControlEdit(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void completionAdd(const string &text);
	virtual void completionClear();
	virtual void __setOption(const string &op, const string &value);
};

#endif /* HUICONTROLEDIT_H_ */
