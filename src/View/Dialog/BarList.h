/*
 * BarList.h
 *
 *  Created on: 04.12.2012
 *      Author: michi
 */

#ifndef BARLIST_H_
#define BARLIST_H_

#include "../../lib/hui/hui.h"
#include "../../Data/AudioFile.h"

class BarList : public HuiEventHandler
{
public:
	BarList(CHuiWindow *_dlg, const string &_id, const string &_id_add, const string &_id_add_pause, const string &_id_delete);
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

	void SetBar(Array<Bar> *bar, int sample_rate);

public:
	CHuiWindow *dlg;
	string id;
	string id_add, id_add_pause, id_delete;
	Array<Bar> *bar;
	int sample_rate;
};

#endif /* BARLIST_H_ */
