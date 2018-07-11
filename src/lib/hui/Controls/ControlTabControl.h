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
	~ControlTabControl() override;
	string getString() override;
	void __setString(const string &str) override;
	void __setInt(int i) override;
	int getInt() override;
	void __addString(const string &str) override;
	void __removeString(int row) override;
	void __changeString(int row, const string &str) override;
	void __setOption(const string &op, const string &value);

	void add(Control *child, int x, int y) override;
	void addPage(const string &str);

	int cur_page;
};

};

#endif /* CONTROLTABCONTROL_H_ */
