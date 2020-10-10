/*
 * ActionTrackAddSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKADDSAMPLE_H_
#define ACTIONTRACKADDSAMPLE_H_

#include "../../Action.h"

class Sample;
class SampleRef;
class TrackLayer;

class ActionTrackAddSample: public Action {
public:
	ActionTrackAddSample(TrackLayer *l, int pos, shared<Sample> sample);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	shared<Sample> sample;
	shared<SampleRef> ref;
	int pos;
};

#endif /* ACTIONTRACKADDSAMPLE_H_ */
