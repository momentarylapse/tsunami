/*
 * SubDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SUBDIALOG_H_
#define SUBDIALOG_H_

#include "../../lib/hui/hui.h"
class Slider;
class Track;
class SampleRef;

class SubDialog: public HuiWindow
{
public:
	SubDialog(HuiWindow *_parent, bool _allow_parent, SampleRef *s);
	virtual ~SubDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnName();
	void OnMute();
	void OnLevelTrack();
	void OnVolume();
	void OnRepNum();
	void OnRepDelay();

	SampleRef *sample;
	Slider *volume_slider;
};

#endif /* SUBDIALOG_H_ */
