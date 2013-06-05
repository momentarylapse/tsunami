/*
 * BarList.h
 *
 *  Created on: 04.12.2012
 *      Author: michi
 */

#ifndef BARLIST_H_
#define BARLIST_H_

#include "../../lib/hui/hui.h"
class Track;

class BarList : public HuiEventHandler
{
public:
	BarList(HuiWindow *_dlg, const string &_id, const string &_id_add, const string &_id_add_pause, const string &_id_delete);
	virtual ~BarList();

	void FillList();
	void OnList();
	void OnListSelect();
	void OnListEdit();
	void OnAdd();
	void OnAddPause();
	void OnDelete();

	void AddNewBar();
	void ExecuteBarDialog(int index);

	void SetTrack(Track *t);

public:
	HuiWindow *dlg;
	string id;
	string id_add, id_add_pause, id_delete;
	Track *track;
};

#endif /* BARLIST_H_ */
