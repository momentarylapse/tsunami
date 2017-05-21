/*
 * LayerConsole.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_LAYERCONSOLE_H_
#define SRC_VIEW_SIDEBAR_LAYERCONSOLE_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"
class Song;

class LayerConsole: public SideBarConsole, public Observer
{
public:
	LayerConsole(Song *s, AudioView *view);
	virtual ~LayerConsole();

	void loadData();
	void applyData();

	void onSelect();
	void onEdit();
	void onMove();
	void onAdd();
	void onDelete();
	void onMerge();

	void onEditSong();

	virtual void onUpdate(Observable *o, const string &message);

	Song *song;
	AudioView *view;
};

#endif /* SRC_VIEW_SIDEBAR_LAYERCONSOLE_H_ */
