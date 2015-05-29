/*
 * HuiControlListView.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLLISTVIEW_H_
#define HUICONTROLLISTVIEW_H_

#include "HuiControl.h"


class HuiControlListView : public HuiControl
{
public:
	HuiControlListView(const string &text, const string &id, HuiPanel *panel);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __addString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual void __changeString(int row, const string &str);
	virtual void __removeString(int row);
	virtual string getCell(int row, int column);
	virtual void __setCell(int row, int column, const string &str);
	virtual Array<int> getMultiSelection();
	virtual void __setSelection(Array<int> &sel);
	virtual void __reset();
	virtual void __setOption(const string &op, const string &value);

#ifdef HUI_API_GTK
	Array<GtkTreeIter> _item_;
#endif
};

#endif /* HUICONTROLLISTVIEW_H_ */
