/*
 * SubDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SUBDIALOG_H_
#define SUBDIALOG_H_

#include "SideBar.h"
class Slider;
class AudioFile;
class Track;
class SampleRef;
class AudioView;

class SubDialog: public SideBarConsole, public Observer
{
public:
	SubDialog(AudioView *view, AudioFile *audio);
	virtual ~SubDialog();

	void LoadData();
	void ApplyData();

	void OnName();
	void OnMute();
	void OnLevelTrack();
	void OnVolume();
	void OnRepNum();
	void OnRepDelay();

	virtual void OnUpdate(Observable *o, const string &message);

	AudioView *view;
	AudioFile *audio;
	Track *track;
	SampleRef *sample;
	Slider *volume_slider;
};

#endif /* SUBDIALOG_H_ */
