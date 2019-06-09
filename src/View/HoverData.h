/*
 * HoverData.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_HOVERDATA_H_
#define SRC_VIEW_HOVERDATA_H_

#include "../lib/base/base.h"
#include "../Data/Range.h"

class ViewNode;
class AudioViewTrack;
class AudioViewLayer;
class Track;
class TrackLayer;
class SampleRef;
class MidiNote;
class TrackMarker;
class Bar;
class SongSelection;
enum class NoteModifier;

class HoverData
{
public:
	ViewNode *node;
	AudioViewLayer *vlayer;
	TrackLayer *layer;
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
	int pitch;
	int clef_position;
	NoteModifier modifier;
	int index;

	enum class Type {
		NONE,
		SOME_NODE,
		BACKGROUND,
		PLAYBACK_CURSOR,
		PLAYBACK_RANGE,
		TIME,
		LAYER,
		LAYER_BODY, // dummy for is_in()
		SAMPLE,
		MIDI_NOTE,
		MIDI_PITCH,
		CLEF_POSITION,
		MARKER,
		BAR,
		BAR_GAP,
		SCROLLBAR_MIDI,
		CURVE_POINT,
		CURVE_POINT_NONE,
	};
	Type type;

	HoverData();
	bool allow_auto_scroll() const;
	bool is_in(Type type) const;
	void clear();

	SongSelection to_song_sel() const;
};

bool hover_changed(HoverData &hover, HoverData &hover_old);

#endif /* SRC_VIEW_HOVERDATA_H_ */
