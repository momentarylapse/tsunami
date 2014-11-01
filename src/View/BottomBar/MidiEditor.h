/*
 * MidiEditor.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIEDITOR_H_
#define MIDIEDITOR_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"
#include "../../lib/math/math.h"

class AudioFile;
class AudioView;

class MidiEditor : public BottomBarConsole, public Observer
{
public:
	MidiEditor(AudioView *view, AudioFile *audio);
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

	void clear();
	void setTrack(Track *t);

	void onAdd();


	string id_inner;

	AudioView *view;
	Track *track;
	AudioFile *audio;
	Array<HuiPanel*> panels;
};

#endif /* MIDIEDITOR_H_ */
