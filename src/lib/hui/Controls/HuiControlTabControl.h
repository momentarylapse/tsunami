/*
 * HuiControlTabControl.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLTABCONTROL_H_
#define HUICONTROLTABCONTROL_H_

#include "HuiControl.h"


class HuiControlTabControl : public HuiControl
{
public:
	HuiControlTabControl(const string &text, const string &id, HuiWindow *win);
	virtual ~HuiControlTabControl();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void __SetInt(int i);
	virtual int GetInt();

	int cur_page;
};

#endif /* HUICONTROLTABCONTROL_H_ */
