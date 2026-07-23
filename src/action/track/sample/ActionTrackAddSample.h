/*
 * ActionTrackAddSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKADDSAMPLE_H_
#define ACTIONTRACKADDSAMPLE_H_

#include <lib/history/Action.h>

namespace tsunami {

struct Sample;
struct SampleRef;
struct TrackLayer;

class ActionTrackAddSample: public history::Action {
public:
	ActionTrackAddSample(TrackLayer *l, int pos, shared<Sample> sample);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	shared<Sample> sample;
	shared<SampleRef> ref;
	int pos;
};

}

#endif /* ACTIONTRACKADDSAMPLE_H_ */
