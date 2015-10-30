/*
 * BarsConsole.h
 *
 *  Created on: 21.10.2015
 *      Author: michi
 */

#ifndef BARSCONSOLE_H_
#define BARSCONSOLE_H_


#include "SideBar.h"
#include "../../Stuff/Observer.h"
#include "../../Data/Range.h"
class BarList;
class Song;
class AudioView;

class BarsConsole: public SideBarConsole, public Observer
{
public:
	BarsConsole(Song *song, AudioView *view);
	virtual ~BarsConsole();

	void updateMessage();
	void onCreateTimeTrack();

	void fillList();
	void onList();
	void onListSelect();
	void onListEdit();
	void onAdd();
	void onAddPause();
	void onDelete();
	void onEdit();
	void onScale();

	void addNewBar();

	void selectToView();
	void selectFromView();
	void performScale();

	void onEditSong();

	virtual void onUpdate(Observable *o, const string &message);

	Song *song;
	string id;
	string id_add, id_add_pause, id_delete, id_edit, id_scale, id_link;
	AudioView *view;

	bool scaling;
	bool scaling_change;
	Array<int> scaling_sel;
	Range scaling_range_orig;
};

#endif /* BARSCONSOLE_H_ */
