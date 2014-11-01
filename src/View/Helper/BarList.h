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
	BarList(HuiPanel *_panel, const string &_id, const string &_id_add, const string &_id_add_pause, const string &_id_delete);
	virtual ~BarList();

	void fillList();
	void onList();
	void onListSelect();
	void onListEdit();
	void onAdd();
	void onAddPause();
	void onDelete();

	void addNewBar();
	void executeBarDialog(int index);

	void setTrack(Track *t);

public:
	HuiPanel *panel;
	string id;
	string id_add, id_add_pause, id_delete;
	Track *track;
};

#endif /* BARLIST_H_ */
