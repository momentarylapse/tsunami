/*
 * SampleRefDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SAMPLEREFDIALOG_H_
#define SAMPLEREFDIALOG_H_

#include "SideBar.h"
class AudioFile;
class Track;
class SampleRef;
class AudioView;

class SampleRefDialog: public SideBarConsole, public Observer
{
public:
	SampleRefDialog(AudioView *view, AudioFile *audio);
	virtual ~SampleRefDialog();

	void loadData();
	void applyData();

	void onName();
	void onMute();
	void onLevelTrack();
	void onVolume();
	void onRepNum();
	void onRepDelay();

	virtual void onUpdate(Observable *o, const string &message);

	AudioView *view;
	AudioFile *audio;
	Track *track;
	SampleRef *sample;
};

#endif /* SAMPLEREFDIALOG_H_ */
