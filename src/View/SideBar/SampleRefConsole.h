/*
 * SampleRefConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SAMPLEREFCONSOLE_H_
#define SAMPLEREFCONSOLE_H_

#include "SideBar.h"
class Track;
class SampleRef;

class SampleRefConsole: public SideBarConsole
{
public:
	SampleRefConsole(Session *session);
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

	void onViewCurSampleChange();
	void onUpdate();

	Track *track;
	SampleRef *sample;
};

#endif /* SAMPLEREFCONSOLE_H_ */
