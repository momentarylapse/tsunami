/*
 * ControlTabControl.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLTABCONTROL_H_
#define CONTROLTABCONTROL_H_

#include "Control.h"

namespace hui
{

class ControlTabControl : public Control
{
public:
	ControlTabControl(const string &text, const string &id, Panel *panel);
	~ControlTabControl();
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual void __addString(const string &str);
	virtual void __removeString(int row);
	virtual void __setOption(const string &op, const string &value);

	virtual void add(Control *child, int x, int y);
	void addPage(const string &str);

	int cur_page;
};

};

#endif /* CONTROLTABCONTROL_H_ */
