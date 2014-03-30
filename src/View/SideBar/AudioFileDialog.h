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
class AudioView;

class AudioFileDialog: public SideBarConsole, public Observer
{
public:
	AudioFileDialog(AudioView *v, AudioFile *a);
	virtual ~AudioFileDialog();

	void LoadData();
	void ApplyData();

	void OnTrackList();
	void OnTagsSelect();
	void OnTagsEdit();
	void OnAddTag();
	void OnDeleteTag();
	void OnLevelsSelect();
	void OnLevelsEdit();
	void OnAddLevel();
	void OnDeleteLevel();

	virtual void OnUpdate(Observable *o, const string &message);

	AudioFile *audio;
	AudioView *view;
	BarList *bar_list;
};

#endif /* AUDIOFILEDIALOG_H_ */
