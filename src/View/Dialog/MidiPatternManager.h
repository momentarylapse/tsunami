/*
 * MidiPatternManager.h
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#ifndef MIDIPATTERNMANAGER_H_
#define MIDIPATTERNMANAGER_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class AudioFile;
class MidiPattern;
class MidiNote;

class MidiPatternManager : public HuiDialog, public Observer
{
public:
	MidiPatternManager(AudioFile *a, HuiWindow* _parent, bool _allow_parent);
	virtual ~MidiPatternManager();

	void FillList();

	void OnListSelect();
	void OnAreaMouseMove();
	void OnAreaLeftButtonDown();
	void OnAreaLeftButtonUp();
	void OnAreaDraw();
	void OnAdd();
	//void OnImportFromFile();
	//void OnInsert();
	//void OnCreateFromSelection();
	void OnDelete();
	void OnClose();

	void SetCurPattern(MidiPattern *p);

	void DrawNote(HuiPainter *c, MidiNote &n, bool hover);

	virtual void OnUpdate(Observable *o);

	AudioFile *audio;
	MidiPattern *cur_pattern;
	Array<string> icon_names;

	int pitch_min, pitch_max;

	int cur_pitch, cur_time;
	int hover_note;
	int area_width, area_height;

	bool creating_new_note;
	int new_time_start;
	MidiNote *new_note;

	int x2time(int x);
	int y2pitch(int y);
	float time2x(int t);
	float pitch2y(int p);
};


#endif /* MIDIPATTERNMANAGER_H_ */
