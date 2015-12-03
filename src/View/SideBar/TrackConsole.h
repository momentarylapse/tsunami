/*
 * TrackConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef TRACKCONSOLE_H_
#define TRACKCONSOLE_H_


#include "SideBar.h"
#include "../../Stuff/Observer.h"
class Track;
class Slider;
class AudioView;

class TrackConsole: public SideBarConsole, public Observer
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
	void onStrings();
	void onString();

	void onEditSong();
	void onEditFx();
	void onEditCurves();
	void onEditMidi();
	void onEditMidiFx();
	void onEditSynth();
	void onEditBars();

	void setTrack(Track *t);

	virtual void onUpdate(Observable *o, const string &message);

	AudioView *view;
	Track *track;

	int num_strings;
};

#endif /* TRACKCONSOLE_H_ */
