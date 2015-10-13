/*
 * MidiEditor.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIEDITOR_H_
#define MIDIEDITOR_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"
#include "../../lib/math/math.h"

class Song;
class AudioView;
class Track;

class MidiEditor : public SideBarConsole, public Observer
{
public:
	MidiEditor(AudioView *view, Song *audio);
	virtual ~MidiEditor();

	virtual void onUpdate(Observable *o, const string &message);
	void update();

	void onPitch();
	void onScale();
	void onBeatPartition();
	void onMidiModeSelect();
	void onMidiModeNote();
	void onMidiModeChord();
	void onChordType();
	void onChordInversion();
	void onReferenceTrack();

	void onEditTrack();
	void onEditMidiFx();
	void onEditSong();

	void clear();
	void setTrack(Track *t);


	string id_inner;

	AudioView *view;
	Track *track;
	Song *song;
};

#endif /* MIDIEDITOR_H_ */
