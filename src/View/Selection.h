/*
 * Selection.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_SELECTION_H_
#define SRC_VIEW_SELECTION_H_

#include "../lib/base/base.h"
#include "../Data/Range.h"

class AudioViewTrack;
class Track;
class SampleRef;
class MidiNote;
class TrackMarker;
class Bar;

class Selection
{
public:
	int type;
	AudioViewTrack *vtrack;
	Track *track;
	SampleRef *sample;
	MidiNote *note;
	TrackMarker *marker;
	Bar *bar;
	int pos;
	Range range;
	int y0;
	int y1;
	int sample_offset;
	Array<int> barrier;
	int pitch;
	int clef_position, modifier;
	int index;

	enum
	{
		TYPE_NONE,
		TYPE_BACKGROUND,
		TYPE_SELECTION_START,
		TYPE_SELECTION_END,
		TYPE_PLAYBACK,
		TYPE_TIME,
		TYPE_TRACK,
		TYPE_TRACK_HEADER,
		TYPE_TRACK_BUTTON_MUTE,
		TYPE_TRACK_BUTTON_SOLO,
		TYPE_TRACK_BUTTON_EDIT,
		TYPE_TRACK_BUTTON_CURVE,
		TYPE_TRACK_BUTTON_FX,
		TYPE_SAMPLE,
		TYPE_MIDI_NOTE,
		TYPE_MIDI_PITCH,
		TYPE_CLEF_POSITION,
		TYPE_MARKER,
		TYPE_BAR,
		TYPE_SCROLL,
		TYPE_CURVE_POINT,
		TYPE_CURVE_POINT_NONE,
	};

	Selection();
	bool allow_auto_scroll() const;
	bool is_in(int type) const;
	void clear();
};

bool hover_changed(Selection &hover, Selection &hover_old);

#endif /* SRC_VIEW_SELECTION_H_ */
