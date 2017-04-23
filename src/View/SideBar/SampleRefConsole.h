/*
 * SampleRefConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SAMPLEREFCONSOLE_H_
#define SAMPLEREFCONSOLE_H_

#include "SideBar.h"
class Song;
class Track;
class SampleRef;
class AudioView;

class SampleRefConsole: public SideBarConsole, public Observer
{
public:
	SampleRefConsole(AudioView *view, Song *song);
	virtual ~SampleRefConsole();

	void loadData();
	void applyData();

	void onName();
	void onMute();
	void onTrack();
	void onVolume();

	void onEditSong();
	void onEditTrack();
	void onEditSample();

	virtual void onUpdate(Observable *o, const string &message);

	AudioView *view;
	Song *song;
	Track *track;
	SampleRef *sample;
};

#endif /* SAMPLEREFCONSOLE_H_ */
