/*
 * LevelConsole.h
 *
 *  Created on: 03.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_LEVELCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_LEVELCONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"

class AudioFile;
class AudioView;

class LevelConsole : public BottomBarConsole, public Observer
{
public:
	LevelConsole(AudioView *view, AudioFile *audio);
	virtual ~LevelConsole();

	void loadData();

	void onLevelsSelect();
	void onLevelsEdit();
	void onAddLevel();
	void onDeleteLevel();

	virtual void onUpdate(Observable *o, const string &message);

	AudioView *view;
	AudioFile *audio;

};

#endif /* SRC_VIEW_BOTTOMBAR_LEVELCONSOLE_H_ */
