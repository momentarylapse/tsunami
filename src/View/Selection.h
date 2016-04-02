/*
 * Selection.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_SELECTION_H_
#define SRC_VIEW_SELECTION_H_

#include "../lib/base/base.h"

class AudioViewTrack;
class Track;
class SampleRef;

class Selection
{
public:
	int type;
	AudioViewTrack *vtrack;
	Track *track;
	SampleRef *sample;
	int pos;
	int sample_offset;
	Array<int> barrier;
	Track *show_track_controls;
	int pitch;
	int clef_position, modifier;
	int index;

	enum
	{
		TYPE_NONE,
		TYPE_SELECTION_START,
		TYPE_SELECTION_END,
		TYPE_PLAYBACK,
		TYPE_TIME,
		TYPE_TRACK,
		TYPE_TRACK_HANDLE,
		TYPE_MUTE,
		TYPE_SOLO,
		TYPE_SAMPLE,
		TYPE_MIDI_NOTE,
		TYPE_MIDI_PITCH,
		TYPE_MARKER,
		TYPE_SCROLL,
		TYPE_CURVE_POINT,
	};

	Selection();
	bool allow_auto_scroll() const;
	void clear();
};

bool hover_changed(Selection &hover, Selection &hover_old);

#endif /* SRC_VIEW_SELECTION_H_ */
