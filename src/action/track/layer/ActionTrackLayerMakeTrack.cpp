/*
 * ActionTrackLayerMakeTrack.cpp
 *
 *  Created on: 14.08.2018
 *      Author: michi
 */

#include "ActionTrackLayerMakeTrack.h"
#include "ActionTrackLayerDelete.h"
#include "../ActionTrackAdd.h"
#include "../../../data/base.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/Song.h"
#include "../../../module/synthesizer/Synthesizer.h"
#include <utility>

namespace tsunami {

class ActionLayerMoveData : public Action {
public:
	ActionLayerMoveData(TrackLayer *_origin, TrackLayer *_dest) {
		origin = _origin;
		dest = _dest;
	}
	void *execute(Data *d) override {
		origin->buffers.exchange(dest->buffers);
		origin->midi.exchange(dest->midi);
		origin->samples.exchange(dest->samples);
		std::swap(origin, dest);
		return nullptr;
	}
	void undo(Data *d) override {
		execute(d);
	}
	TrackLayer *origin, *dest;
};

int get_layer_index(TrackLayer *layer) {
	return weak(layer->track->layers).find(layer);
}

SignalType effective_type(Track *t) {
	if (t->type == SignalType::Audio and t->channels == 1)
		return SignalType::AudioMono;
	if (t->type == SignalType::Audio and t->channels == 2)
		return SignalType::AudioStereo;
	return t->type;
}

ActionTrackLayerMakeTrack::ActionTrackLayerMakeTrack(TrackLayer *_layer) {
	layer = _layer;
}

void ActionTrackLayerMakeTrack::build(Data *d) {
	Track *orig = layer->track;
	Track *t = new Track(orig->song, effective_type(layer->track), (Synthesizer*)orig->synth->copy());
	t->layers.add(new TrackLayer(t));
	t->instrument = orig->instrument;
	t->volume = orig->volume;
	t->panning = orig->panning;
	add_sub_action(new ActionTrackAdd(t, orig->get_index() + 1), d);
	add_sub_action(new ActionLayerMoveData(layer, t->layers[0].get()), d);
	add_sub_action(new ActionTrackLayerDelete(layer->track, get_layer_index(layer)), d);
}

}

