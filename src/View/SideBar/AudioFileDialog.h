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
class Slider;
class BarList;

class AudioFileDialog: public SideBarConsole, public Observer
{
public:
	AudioFileDialog(AudioFile *a);
	virtual ~AudioFileDialog();

	void LoadData();
	void ApplyData();

	void OnVolume();
	void OnTrackList();
	void OnTagsSelect();
	void OnTagsEdit();
	void OnAddTag();
	void OnDeleteTag();

	virtual void OnUpdate(Observable *o, const string &message);

	AudioFile *audio;
	Slider *volume_slider;
	BarList *bar_list;
};

#endif /* AUDIOFILEDIALOG_H_ */
