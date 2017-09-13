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

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(const SongSelection &_sel, int _layer_no) :
	sel(_sel)
{
	layer_no = _layer_no;
}

void ActionTrackSampleFromSelection::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	for (Track *t: s->tracks)
		if (sel.has(t)){
			if (t->type == t->TYPE_AUDIO)
				CreateSamplesFromTrackAudio(t, sel, layer_no);
			else if (t->type == t->TYPE_MIDI)
				CreateSamplesFromTrackMidi(t, sel);
		}
}

void ActionTrackSampleFromSelection::CreateSamplesFromTrackAudio(Track *t, const SongSelection &sel, int layer_no)
{
	TrackLayer &l = t->layers[layer_no];
	foreachib(AudioBuffer &b, l.buffers, bi)
		if (sel.range.covers(b.range())){
			addSubAction(new ActionTrackPasteAsSample(t, b.offset, b, false), t->song);
			addSubAction(new ActionTrack__DeleteBuffer(t, layer_no, bi), t->song);
		}
}

void ActionTrackSampleFromSelection::CreateSamplesFromTrackMidi(Track *t, const SongSelection &selo)
{
	MidiData midi = t->midi.getNotesBySelection(sel);
	midi.samples = sel.range.length;
	if (midi.num == 0)
		return;
	for (auto n: midi)
		n->range.offset -= sel.range.offset;
	addSubAction(new ActionTrackPasteAsSample(t, sel.range.offset, midi, false), t->song);
	foreachib(MidiNote *n, t->midi, i)
		if (sel.has(n))
			addSubAction(new ActionTrackDeleteMidiNote(t, i), t->song);
}
