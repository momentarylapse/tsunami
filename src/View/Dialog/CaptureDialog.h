/*
 * CaptureDialog.h
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#ifndef CAPTUREDIALOG_H_
#define CAPTUREDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../Data/AudioFile.h"
#include "../Helper/PeakMeter.h"
#include "../../Stuff/Observer.h"

class AudioView;

class CaptureDialog : public HuiWindow, public Observer
{
public:
	CaptureDialog(HuiWindow *_parent, bool _allow_parent, AudioFile *a);
	virtual ~CaptureDialog();

	void onTypeAudio();
	void onTypeMidi();
	void onTarget();
	void onStart();
	void onDelete();
	void onPause();
	void onOk();
	void onClose();
	bool insert();

	void fillTrackList();

	void onUpdate(Observable *o, const string &message);

	void setTarget(int index);
	void setType(int type);

	AudioFile *audio;
	AudioView *view;
	PeakMeter *peak_meter;
	Synthesizer *temp_synth;
	int type;
};

#endif /* CAPTUREDIALOG_H_ */
