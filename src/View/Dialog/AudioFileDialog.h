/*
 * AudioFileDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOFILEDIALOG_H_
#define AUDIOFILEDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"
class AudioFile;
class Slider;
class BarList;
class FxList;

class AudioFileDialog: public HuiPanel, public Observer
{
public:
	AudioFileDialog(AudioFile *a);
	virtual ~AudioFileDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnVolume();
	void OnTrackList();
	void OnTagsSelect();
	void OnTagsEdit();
	void OnAddTag();
	void OnDeleteTag();

	virtual void OnUpdate(Observable *o, const string &message);

	AudioFile *audio;
	Slider *volume_slider;
	FxList *fx_list;
	BarList *bar_list;
};

#endif /* AUDIOFILEDIALOG_H_ */
