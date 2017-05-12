/*
 * ControlListView.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLLISTVIEW_H_
#define CONTROLLISTVIEW_H_

#include "Control.h"

namespace hui
{

class ControlListView : public Control
{
public:
	ControlListView(const string &text, const string &id, Panel *panel);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __addString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual void __changeString(int row, const string &str);
	virtual void __removeString(int row);
	virtual string getCell(int row, int column);
	virtual void __setCell(int row, int column, const string &str);
	virtual Array<int> getSelection();
	virtual void __setSelection(Array<int> &sel);
	virtual void __reset();
	virtual void __setOption(const string &op, const string &value);

	bool allow_change_messages;
	int row_target;
};

};

#endif /* CONTROLLISTVIEW_H_ */
