/*
 * SampleRefDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SAMPLEREFDIALOG_H_
#define SAMPLEREFDIALOG_H_

#include "SideBar.h"
class Song;
class Track;
class SampleRef;
class AudioView;

class SampleRefDialog: public SideBarConsole, public Observer
{
public:
	SampleRefDialog(AudioView *view, Song *song);
	virtual ~SampleRefDialog();

	void loadData();
	void applyData();

	void onName();
	void onMute();
	void onLevelTrack();
	void onVolume();
	void onRepNum();
	void onRepDelay();

	void onEditSong();
	void onEditTrack();

	virtual void onUpdate(Observable *o, const string &message);

	AudioView *view;
	Song *song;
	Track *track;
	SampleRef *sample;
};

#endif /* SAMPLEREFDIALOG_H_ */
