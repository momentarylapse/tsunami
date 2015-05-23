/*
 * LevelConsole.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_LEVELCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_LEVELCONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"
class AudioFile;

class LevelConsole: public BottomBarConsole, public Observer
{
public:
	LevelConsole(AudioFile *a, AudioView *view);
	virtual ~LevelConsole();

	void loadData();
	void applyData();

	void onSelect();
	void onEdit();
	void onAdd();
	void onDelete();

	virtual void onUpdate(Observable *o, const string &message);

	AudioFile *audio;
	AudioView *view;
};

#endif /* SRC_VIEW_BOTTOMBAR_LEVELCONSOLE_H_ */
