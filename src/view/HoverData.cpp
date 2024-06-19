/*
 * Selection.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "HoverData.h"
#include "audioview/graph/AudioViewLayer.h"
#include "audioview/graph/AudioViewTrack.h"
#include "../data/midi/MidiData.h"

namespace tsunami {

HoverData::HoverData() {
	clear();
}

bool HoverData::allow_auto_scroll() const {
	return (type == Type::SAMPLE) or (type == Type::PLAYBACK_CURSOR) or (type == Type::MIDI_PITCH) or (type == Type::CLEF_POSITION);
}

Track *HoverData::track() const {
	if (vlayer)
		return vlayer->track();
	return nullptr;
}

AudioViewTrack *HoverData::vtrack() const {
	if (vlayer)
		return vlayer->vtrack();
	return nullptr;
}

TrackLayer *HoverData::layer() const {
	if (vlayer)
		return vlayer->layer;
	return nullptr;
}

void HoverData::clear() {
	type = Type::NONE;
	node = nullptr;
	vlayer = nullptr;
	sample = nullptr;
	note = nullptr;
	marker = nullptr;
	bar = nullptr;
	index = 0;
	pos = 0;
	pos_snap = 0;
	range = Range::NONE;
	y0 = y1 = 0;
}

}

