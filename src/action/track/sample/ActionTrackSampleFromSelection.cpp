/*
 * ActionTrackSampleFromSelection.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackSampleFromSelection.h"
#include "ActionTrackPasteAsSample.h"
#include "../buffer/ActionTrackEditBuffer.h"
#include "../midi/ActionTrackDeleteMidiNote.h"
#include "../../../data/base.h"
#include "../../../data/SongSelection.h"
#include "../../../data/Song.h"
#include "../../../data/TrackLayer.h"
#include "../buffer/ActionTrack__DeleteBuffer.h"

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(const SongSelection &_sel, bool _auto_delete) :
	sel(_sel)
{
	auto_delete = _auto_delete;
}

void ActionTrackSampleFromSelection::build(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	for (TrackLayer *l: s->layers())
		if (sel.has(l)) {
			if (l->type == SignalType::AUDIO)
				CreateSamplesFromLayerAudio(l);
			else if (l->type == SignalType::MIDI)
				CreateSamplesFromLayerMidi(l);
		}
}

void ActionTrackSampleFromSelection::CreateSamplesFromLayerAudio(TrackLayer *l) {
	foreachib(AudioBuffer &b, l->buffers, bi)
		if (sel.range().covers(b.range())) {
			add_sub_action(new ActionTrackPasteAsSample(l, b.offset, b, auto_delete), l->song());
			add_sub_action(new ActionTrack__DeleteBuffer(l, bi), l->song());
		}
}

void ActionTrackSampleFromSelection::CreateSamplesFromLayerMidi(TrackLayer *l) {
	MidiNoteBuffer midi = l->midi.get_notes_by_selection(sel);
	midi.samples = sel.range().length;
	if (midi.num == 0)
		return;
	for (auto n: midi)
		n->range.offset -= sel.range().offset;
	add_sub_action(new ActionTrackPasteAsSample(l, sel.range().offset, midi, auto_delete), l->song());
	foreachib(MidiNote *n, weak(l->midi), i)
		if (sel.has(n))
			add_sub_action(new ActionTrackDeleteMidiNote(l, i), l->song());
}
