/*
 * SubDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SUBDIALOG_H_
#define SUBDIALOG_H_

#include "SideBar.h"
class AudioFile;
class Track;
class SampleRef;
class AudioView;

class SubDialog: public SideBarConsole, public Observer
{
public:
	SubDialog(AudioView *view, AudioFile *audio);
	virtual ~SubDialog();

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

#endif /* SUBDIALOG_H_ */
