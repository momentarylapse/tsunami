/*
 * LevelConsole.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_LEVELCONSOLE_H_
#define SRC_VIEW_SIDEBAR_LEVELCONSOLE_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"
class Song;

class LevelConsole: public SideBarConsole, public Observer
{
public:
	LevelConsole(Song *s, AudioView *view);
	virtual ~LevelConsole();

	void loadData();
	void applyData();

	void onSelect();
	void onEdit();
	void onAdd();
	void onDelete();
	void onMerge();

	void onEditSong();

	virtual void onUpdate(Observable *o, const string &message);

	Song *song;
	AudioView *view;
};

#endif /* SRC_VIEW_SIDEBAR_LEVELCONSOLE_H_ */
