/*
 * TrackConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef TRACKCONSOLE_H_
#define TRACKCONSOLE_H_


#include "SideBar.h"
class Track;
class Slider;
class AudioView;

class TrackConsole: public SideBarConsole
{
public:
	TrackConsole(AudioView *view);
	virtual ~TrackConsole();

	void loadData();
	void applyData();

	void onName();
	void onVolume();
	void onPanning();
	void onInstrument();
	void onEditTuning();
	void onSelectSynth();

	void onEditSong();
	void onEditFx();
	void onEditCurves();
	void onEditMidi();
	void onEditMidiFx();
	void onEditSynth();

	void setTrack(Track *t);

	void onViewCurTrackChange();
	void onUpdate();

	AudioView *view;
	Track *track;
};

#endif /* TRACKCONSOLE_H_ */
