/*
 * MidiEditor.h
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#ifndef MIDIEDITOR_H_
#define MIDIEDITOR_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class AudioFile;
class MidiPattern;
class MidiNote;
class Track;
class Bar;
class Range;

class MidiEditor : public HuiDialog, public Observer
{
public:
	MidiEditor(HuiWindow* _parent, bool _allow_parent, AudioFile *a, Track *t);
	virtual ~MidiEditor();

	void OnAreaMouseMove();
	void OnAreaLeftButtonDown();
	void OnAreaLeftButtonUp();
	void OnAreaDraw();
	void OnBeatPartition();
	void OnPlay();
	void OnStop();
	void OnDelete();
	void OnClose();

	void CreateParts();

	void DrawNote(HuiPainter *c, MidiNote &n, bool hover);

	virtual void OnUpdate(Observable *o);

	AudioFile *audio;
	Track *track;
	Array<Bar> bars;
	Array<Range> parts;

	int pitch_min, pitch_max;
	int beat_partition;

	int cur_pitch, cur_sample, cur_part;
	int hover_note;
	int area_width, area_height;

	bool creating_new_note;
	int new_part_start;
	MidiNote *new_note;

	int x2sample(int x);
	int y2pitch(int y);
	float sample2x(int s);
	float pitch2y(int p);
};


#endif /* MIDIEDITOR_H_ */
