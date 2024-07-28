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

namespace tsunami {

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
		None,
		SomeNode,
		Background,
		PlaybackCursor,
		PlaybackRange,
		Time,
		Layer,
		LayerBody, // dummy for is_in()
		Sample,
		MidiNote,
		MidiPitch,
		ClefPosition,
		Marker,
		Bar,
		BarGap,
		ScrollbarMidi,
		CurvePoint,
		CurvePointNone,
		Module,
		ModulePortIn,
		ModulePortOut,
		ModuleCable,
	};
	Type type;

	HoverData();
	bool allow_auto_scroll() const;
	void clear();

	SongSelection to_song_sel() const;
};

bool hover_changed(HoverData &hover, HoverData &hover_old);

}

#endif /* SRC_VIEW_HOVERDATA_H_ */
