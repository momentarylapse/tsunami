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

	void addNewBar();
	void executeBarDialog(int index);

	void selectToView();
	void selectFromView();

	void onEditSong();

	virtual void onUpdate(Observable *o, const string &message);

	Song *song;
	string id;
	string id_add, id_add_pause, id_delete, id_edit, id_link;
	AudioView *view;
};

#endif /* BARSCONSOLE_H_ */
