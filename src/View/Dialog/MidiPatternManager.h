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

class MidiPatternManager : public HuiDialog, public Observer
{
public:
	MidiPatternManager(AudioFile *a, HuiWindow* _parent, bool _allow_parent);
	virtual ~MidiPatternManager();

	void FillList();

	void OnListSelect();
	void OnAreaMouseMove();
	void OnAreaClick();
	void OnAreaDraw();
	void OnAdd();
	//void OnImportFromFile();
	//void OnInsert();
	//void OnCreateFromSelection();
	void OnDelete();
	void OnClose();

	void SetCurPattern(MidiPattern *p);

	virtual void OnUpdate(Observable *o);

	AudioFile *audio;
	MidiPattern *cur_pattern;
	Array<string> icon_names;

	int pitch_min, pitch_max;

	int cur_pitch, cur_time;
	int area_width, area_height;

	int x2beat(int x);
	int y2pitch(int y);
	float beat2x(int b);
	float pitch2y(int p);
};


#endif /* MIDIPATTERNMANAGER_H_ */
