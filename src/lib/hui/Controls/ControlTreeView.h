/*
 * ControlTreeView.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef CONTROLTREEVIEW_H_
#define CONTROLTREEVIEW_H_

#include "Control.h"

namespace hui
{

class ControlTreeView : public Control
{
public:
	ControlTreeView(const string &title, const string &id, Panel *panel);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __addString(const string &str);
	virtual void __setInt(int i);
	virtual int getInt();
	virtual void __addChildString(int parent_row, const string &str);
	virtual void __changeString(int row, const string &str);
	virtual void __removeString(int row);
	virtual string getCell(int row, int column);
	virtual void __setCell(int row, int column, const string &str);
	virtual Array<int> getSelection();
	virtual void __setSelection(Array<int> &sel);
	virtual void __reset();
	virtual void expand(int row, bool expand);
	virtual void expandAll(bool expand);
	virtual bool isExpanded(int row);
	virtual void __setOption(const string &op, const string &value);

#ifdef HUI_API_GTK
	Array<GtkTreeIter> _item_;
#endif
};

};

#endif /* CONTROLTREEVIEW_H_ */
