/*
 * AudioFileConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOFILECONSOLE_H_
#define AUDIOFILECONSOLE_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"
class AudioFile;
class BarList;

class AudioFileConsole: public SideBarConsole, public Observer
{
public:
	AudioFileConsole(AudioFile *a);
	virtual ~AudioFileConsole();

	void loadData();
	void applyData();

	void onSamplerate();
	void onFormat();
	void onCompression();
	void onTrackList();
	void onTagsSelect();
	void onTagsEdit();
	void onAddTag();
	void onDeleteTag();

	virtual void onUpdate(Observable *o, const string &message);

	AudioFile *audio;
	BarList *bar_list;
};

#endif /* AUDIOFILECONSOLE_H_ */
