/*
 * HuiControlTreeView.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLTREEVIEW_H_
#define HUICONTROLTREEVIEW_H_

#include "HuiControl.h"


class HuiControlTreeView : public HuiControl
{
public:
	HuiControlTreeView(const string &title, const string &id, HuiWindow *win);
	virtual ~HuiControlTreeView();
	virtual string GetString();
	virtual void __SetString(const string &str);
	virtual void __AddString(const string &str);
	virtual void __SetInt(int i);
	virtual int GetInt();
	virtual void __AddChildString(int parent_row, const string &str);
	virtual void __ChangeString(int row, const string &str);
	virtual string GetCell(int row, int column);
	virtual void __SetCell(int row, int column, const string &str);
	virtual Array<int> GetMultiSelection();
	virtual void __SetMultiSelection(Array<int> &sel);
	virtual void __Reset();
	virtual void Expand(int row, bool expand);
	virtual void ExpandAll(bool expand);
	virtual bool IsExpanded(int row);

#ifdef HUI_API_GTK
	Array<GtkTreeIter> _item_;
#endif
};

#endif /* HUICONTROLTREEVIEW_H_ */
