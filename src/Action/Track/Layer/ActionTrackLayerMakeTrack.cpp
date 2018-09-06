/*
 * ActionTrackLayerMakeTrack.cpp
 *
 *  Created on: 14.08.2018
 *      Author: michi
 */

#include "ActionTrackLayerMakeTrack.h"
#include "ActionTrackLayerDelete.h"
#include "../ActionTrackAdd.h"
#include "../../../Data/base.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/Song.h"
#include "../../../Module/Synth/Synthesizer.h"
#include <utility>

class ActionLayerMoveData : public Action
{
public:
	ActionLayerMoveData(TrackLayer *_origin, TrackLayer *_dest)
	{
		origin = _origin;
		dest = _dest;
	}
	void *execute(Data *d) override
	{
		origin->buffers.exchange(dest->buffers);
		origin->midi.exchange(dest->midi);
		origin->samples.exchange(dest->samples);
		std::swap(origin, dest);
		return nullptr;
	}
	void undo(Data *d) override
	{
		execute(d);
	}
	TrackLayer *origin, *dest;
};

int get_layer_index(TrackLayer *layer)
{
	return layer->track->layers.find(layer);
}

SignalType effective_type(Track *t)
{
	if (t->type == SignalType::AUDIO and t->channels == 1)
		return SignalType::AUDIO_MONO;
	if (t->type == SignalType::AUDIO and t->channels == 2)
		return SignalType::AUDIO_STEREO;
	return t->type;
}

ActionTrackLayerMakeTrack::ActionTrackLayerMakeTrack(TrackLayer *_layer)
{
	layer = _layer;
}

void ActionTrackLayerMakeTrack::build(Data *d)
{
	Track *orig = layer->track;
	Track *t = new Track(effective_type(layer->track), (Synthesizer*)orig->synth->copy());
	t->instrument = orig->instrument;
	t->volume = orig->volume;
	t->panning = orig->panning;
	addSubAction(new ActionTrackAdd(t, orig->get_index() + 1), d);
	addSubAction(new ActionLayerMoveData(layer, t->layers[0]), d);
	addSubAction(new ActionTrackLayerDelete(layer->track, get_layer_index(layer)), d);
}

