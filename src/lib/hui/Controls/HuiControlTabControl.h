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
	HuiControlTabControl(const string &text, const string &id, HuiPanel *panel);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual void __addString(const string &str);
	virtual void __removeString(int row);
	virtual void __setOption(const string &op, const string &value);

	virtual void add(HuiControl *child, int x, int y);
	void addPage(const string &str);

	int cur_page;
};

#endif /* HUICONTROLTABCONTROL_H_ */
