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
	virtual ~HuiControlEdit();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void CompletionAdd(const string &text);
	virtual void CompletionClear();
};

#endif /* HUICONTROLEDIT_H_ */
