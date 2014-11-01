/*
 * AudioFileDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOFILEDIALOG_H_
#define AUDIOFILEDIALOG_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"
class AudioFile;
class BarList;

class AudioFileDialog: public SideBarConsole, public Observer
{
public:
	AudioFileDialog(AudioFile *a);
	virtual ~AudioFileDialog();

	void loadData();
	void applyData();

	void onTrackList();
	void onTagsSelect();
	void onTagsEdit();
	void onAddTag();
	void onDeleteTag();

	virtual void onUpdate(Observable *o, const string &message);

	AudioFile *audio;
	BarList *bar_list;
};

#endif /* AUDIOFILEDIALOG_H_ */
