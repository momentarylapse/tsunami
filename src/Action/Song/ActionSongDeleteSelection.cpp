/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionSongDeleteSelection.h"
#include "../Track/Buffer/ActionTrack__SplitBufferBox.h"
#include "../Track/Buffer/ActionTrack__DeleteBufferBox.h"
#include "../Track/Buffer/ActionTrack__ShrinkBufferBox.h"
#include "../Track/Sample/ActionTrackDeleteSample.h"
#include "../Track/Marker/ActionTrackDeleteMarker.h"
#include "../Track/Midi/ActionTrackDeleteMidiNote.h"
#include "../../Data/SongSelection.h"

ActionSongDeleteSelection::ActionSongDeleteSelection(int _layer_no, const SongSelection &_sel, bool _all_layers) :
	sel(_sel)
{
	layer_no = _layer_no;
	all_layers = _all_layers;
}

void ActionSongDeleteSelection::build(Data *d)
{
	msg_write("del sel.build");
	Song *s = dynamic_cast<Song*>(d);
	for (Track *t: s->tracks){
		if (!sel.has(t))
			continue;

		// buffer boxes
		if (all_layers){
			foreachi(TrackLayer &l, t->layers, li)
				DeleteBuffersFromTrackLayer(s, t, l, sel, li);
		}else{
			DeleteBuffersFromTrackLayer(s, t, t->layers[layer_no], sel, layer_no);
		}



		// subs
		for (int i=t->samples.num-1; i>=0; i--)
			if (sel.has(t->samples[i]))
				addSubAction(new ActionTrackDeleteSample(t->samples[i]), d);

		// midi
		for (int i=t->midi.num-1; i>=0; i--)
			if (sel.has(t->midi[i]))
				addSubAction(new ActionTrackDeleteMidiNote(t, i), d);

		// marker
		for (int i=t->markers.num-1; i>=0; i--)
			if (sel.has(t->markers[i]))
				addSubAction(new ActionTrackDeleteMarker(t, i), d);
	}
}

void ActionSongDeleteSelection::DeleteBuffersFromTrackLayer(Song* a, Track *t, TrackLayer& l, const SongSelection &sel, int layer_no)
{
	int i0 = sel.range.start();
	int i1 = sel.range.end();
	foreachib(BufferBox &b, l.buffers, n){
		int bi0 = b.offset;
		int bi1 = b.offset + b.length;


		if (sel.range.covers(b.range())){
			// b completely inside?
			addSubAction(new ActionTrack__DeleteBufferBox(t, layer_no, n), a);

		}else if (sel.range.is_inside(bi1-1)){
			// overlapping end of b?
			addSubAction(new ActionTrack__ShrinkBufferBox(t, layer_no, n, i0 - bi0), a);

		}else if (sel.range.is_inside(bi0)){
			// overlapping beginning of b?
			addSubAction(new ActionTrack__SplitBufferBox(t, layer_no, n, i1 - bi0), a);
			addSubAction(new ActionTrack__DeleteBufferBox(t, layer_no, n), a);

		}else if (b.range().covers(sel.range)){
			// inside b?
			addSubAction(new ActionTrack__SplitBufferBox(t, layer_no, n, i1 - bi0), a);
			addSubAction(new ActionTrack__SplitBufferBox(t, layer_no, n, i0 - bi0), a);
			addSubAction(new ActionTrack__DeleteBufferBox(t, layer_no, n + 1), a);

		}
		_foreach_it_.update();
	}
}

