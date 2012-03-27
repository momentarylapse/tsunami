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
#include "../../Stuff/PeakMeter.h"
#include "../../Stuff/Observer.h"

class CaptureDialog : public CHuiWindow, public Observer
{
public:
	CaptureDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a);
	virtual ~CaptureDialog();

	void OnDevice();
	void OnStart();
	void OnDelete();
	void OnPause();
	void OnOk();
	void OnClose();
	void Insert();

	void OnUpdate(Observable *o);

	AudioFile *audio;
	PeakMeter *peak_meter;
};

#endif /* CAPTUREDIALOG_H_ */
