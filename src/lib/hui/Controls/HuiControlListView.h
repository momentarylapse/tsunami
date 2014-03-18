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
	virtual ~HuiControlListView();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void __AddString(const string &str);
	virtual void __SetInt(int i);
	virtual int GetInt();
	virtual void __ChangeString(int row, const string &str);
	virtual string GetCell(int row, int column);
	virtual void __SetCell(int row, int column, const string &str);
	virtual Array<int> GetMultiSelection();
	virtual void __SetMultiSelection(Array<int> &sel);
	virtual void __Reset();

#ifdef HUI_API_GTK
	Array<GtkTreeIter> _item_;
#endif
};

#endif /* HUICONTROLLISTVIEW_H_ */
