/*
 * AudioFileConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOFILECONSOLE_H_
#define AUDIOFILECONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"
class AudioFile;
class BarList;

class AudioFileConsole: public BottomBarConsole, public Observer
{
public:
	AudioFileConsole(AudioFile *a, AudioView *view);
	virtual ~AudioFileConsole();

	void loadData();
	void applyData();

	void onTrackList();
	void onTagsSelect();
	void onTagsEdit();
	void onAddTag();
	void onDeleteTag();

	void onLevelsSelect();
	void onLevelsEdit();
	void onAddLevel();
	void onDeleteLevel();

	virtual void onUpdate(Observable *o, const string &message);

	AudioFile *audio;
	AudioView *view;
	BarList *bar_list;
};

#endif /* AUDIOFILECONSOLE_H_ */
