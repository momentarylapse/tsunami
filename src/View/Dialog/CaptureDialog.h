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

class CaptureDialog : public HuiWindow, public Observer
{
public:
	CaptureDialog(HuiWindow *_parent, bool _allow_parent, AudioFile *a);
	virtual ~CaptureDialog();

	void OnTypeAudio();
	void OnTypeMidi();
	void OnStart();
	void OnDelete();
	void OnPause();
	void OnOk();
	void OnClose();
	bool Insert();

	void FillTrackList();

	void OnUpdate(Observable *o);

	AudioFile *audio;
	PeakMeter *peak_meter;
	int type;
};

#endif /* CAPTUREDIALOG_H_ */
