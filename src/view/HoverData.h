/*
 * HoverData.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_HOVERDATA_H_
#define SRC_VIEW_HOVERDATA_H_

#include "../lib/base/base.h"
#include "../data/Range.h"

namespace scenegraph {
	class Node;
}
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

class HoverData {
public:
	scenegraph::Node *node;
	AudioViewLayer *vlayer;
	TrackLayer *layer() const;
	AudioViewTrack *vtrack() const;
	Track *track() const;
	SampleRef *sample;
	MidiNote *note;
	TrackMarker *marker;
	Bar *bar;
	int pos;
	int pos_snap;
	Range range;
	int y0;
	int y1;
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
		MODULE,
		PORT_IN,
		PORT_OUT,
		CABLE,
	};
	Type type;

	HoverData();
	bool allow_auto_scroll() const;
	void clear();

	SongSelection to_song_sel() const;
};

bool hover_changed(HoverData &hover, HoverData &hover_old);

#endif /* SRC_VIEW_HOVERDATA_H_ */
