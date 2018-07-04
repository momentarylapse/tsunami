/*
 * ActionTrackSampleFromSelection.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackSampleFromSelection.h"
#include "ActionTrackPasteAsSample.h"
#include "../Buffer/ActionTrackEditBuffer.h"
#include "../Midi/ActionTrackDeleteMidiNote.h"
#include "../../../Data/SongSelection.h"
#include "../Buffer/ActionTrack__DeleteBuffer.h"

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(const SongSelection &_sel) :
	sel(_sel)
{
}

void ActionTrackSampleFromSelection::build(Data *d)
{
	/*Song *s = dynamic_cast<Song*>(d);
	for (Track *t: s->tracks)
		if (sel.has(t)){
			if (t->type == t->Type::AUDIO)
				CreateSamplesFromTrackAudio(t, sel, layer_no);
			else if (t->type == t->Type::MIDI)
				CreateSamplesFromTrackMidi(t, sel);
		}*/
}

void ActionTrackSampleFromSelection::CreateSamplesFromTrackAudio(TrackLayer *l, const SongSelection &sel)
{
/*	foreachib(AudioBuffer &b, l->buffers, bi)
		if (sel.range.covers(b.range())){
			addSubAction(new ActionTrackPasteAsSample(t, b.offset, b, false), t->song);
			addSubAction(new ActionTrack__DeleteBuffer(t, layer_no, bi), t->song);
		}*/
}

void ActionTrackSampleFromSelection::CreateSamplesFromTrackMidi(Track *t, const SongSelection &selo)
{
	/*MidiNoteBuffer midi = t->midi.getNotesBySelection(sel);
	midi.samples = sel.range.length;
	if (midi.num == 0)
		return;
	for (auto n: midi)
		n->range.offset -= sel.range.offset;
	addSubAction(new ActionTrackPasteAsSample(t, sel.range.offset, midi, false), t->song);
	foreachib(MidiNote *n, t->midi, i)
		if (sel.has(n))
			addSubAction(new ActionTrackDeleteMidiNote(t, i), t->song);*/
}
