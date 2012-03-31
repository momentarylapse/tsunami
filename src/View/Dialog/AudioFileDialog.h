/*
 * AudioFileDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOFILEDIALOG_H_
#define AUDIOFILEDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../Data/AudioFile.h"
#include "FxList.h"

class AudioFileDialog: public CHuiWindow
{
public:
	AudioFileDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a);
	virtual ~AudioFileDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnTrackList();

	Array<Track*> audio_list;

	void RefillAudioList();

	AudioFile *audio;
	FxList *fx_list;
};

#endif /* AUDIOFILEDIALOG_H_ */
