/*
 * ActionTrackDeleteSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackDeleteSample.h"

#include "../../../Data/TrackLayer.h"
#include "../../../Data/SampleRef.h"
#include "../../../Data/Sample.h"
#include "../../Sample/ActionSampleDelete.h"


class ActionTrack__DeleteSample : public Action {
private:
	TrackLayer *layer;
	int index;
	SampleRef *ref;
public:
	ActionTrack__DeleteSample(SampleRef *_ref) {
		layer = _ref->layer;
		index = _ref->get_index();
		ref = nullptr;
	}
	~ActionTrack__DeleteSample() {
		if (ref)
			if (!ref->owner)
				delete ref;
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
		ref = nullptr;
	}
};

ActionTrackDeleteSample::ActionTrackDeleteSample(SampleRef *_ref) {
	ref = _ref;
}

void ActionTrackDeleteSample::build(Data *d) {
	Sample *sample = ref->origin.get();

	add_sub_action(new ActionTrack__DeleteSample(ref), d);

	if (sample->auto_delete and (sample->ref_count == 0))
		add_sub_action(new ActionSampleDelete(sample), d);
}
