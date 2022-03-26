/*
 * ActionTrackDeleteSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackDeleteSample.h"

#include "../../../data/TrackLayer.h"
#include "../../../data/SampleRef.h"
#include "../../../data/Sample.h"
#include "../../sample/ActionSampleDelete.h"


class ActionTrack__DeleteSample : public Action {
private:
	TrackLayer *layer;
	int index;
	shared<SampleRef> ref;
public:
	ActionTrack__DeleteSample(shared<SampleRef> _ref) {
		layer = _ref->layer;
		index = _ref->get_index();
	}
	void *execute(Data* d) override {
		ref = layer->samples[index];
		ref->origin->unref();
		ref->owner = nullptr;

		ref->fake_death();
		layer->samples.erase(index);

		return nullptr;
	}
	void undo(Data* d) override {
		layer->samples.insert(ref, index);
		ref->origin->ref();
		ref->owner = layer->song();
	}
};

ActionTrackDeleteSample::ActionTrackDeleteSample(shared<SampleRef> _ref) {
	ref = _ref;
}

void ActionTrackDeleteSample::build(Data *d) {
	auto sample = ref->origin;

	add_sub_action(new ActionTrack__DeleteSample(ref), d);

	if (sample->auto_delete and (sample->ref_count == 0))
		add_sub_action(new ActionSampleDelete(sample), d);
}
