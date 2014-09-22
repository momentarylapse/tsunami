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

	virtual void OnUpdate(Observable *o, const string &message);
	void update();

	void OnPitch();
	void OnBeatPartition();
	void OnInsertChord();
	void OnChordType();
	void OnChordInversion();

	void Clear();
	void SetTrack(Track *t);

	void OnAdd();


	string id_inner;

	AudioView *view;
	Track *track;
	AudioFile *audio;
	Array<HuiPanel*> panels;
};

#endif /* MIDIEDITOR_H_ */
