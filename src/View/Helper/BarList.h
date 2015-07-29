/*
 * BarList.h
 *
 *  Created on: 04.12.2012
 *      Author: michi
 */

#ifndef BARLIST_H_
#define BARLIST_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"
class Track;
class AudioView;

class BarList : public HuiEventHandler, public Observer
{
public:
	BarList(HuiPanel *_panel, const string &_id, const string &_id_add, const string &_id_add_pause, const string &_id_delete, const string &_id_set_bpm, AudioView *view);
	virtual ~BarList();

	void fillList();
	void onList();
	void onListSelect();
	void onListEdit();
	void onAdd();
	void onAddPause();
	void onDelete();
	void onSetBpm();

	void addNewBar();
	void executeBarDialog(int index);

	void setTrack(Track *t);

	virtual void onUpdate(Observable *o, const string &message);

	void selectToView();
	void selectFromView();

public:
	HuiPanel *panel;
	string id;
	string id_add, id_add_pause, id_delete, id_set_bpm;
	Track *track;
	AudioView *view;
};

#endif /* BARLIST_H_ */
