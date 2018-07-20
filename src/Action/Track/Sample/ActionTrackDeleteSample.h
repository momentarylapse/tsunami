/*
 * ActionTrackDeleteSample.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETESAMPLE_H_
#define ACTIONTRACKDELETESAMPLE_H_

#include "../../Action.h"
class SampleRef;
class TrackLayer;

class ActionTrackDeleteSample : public Action
{
public:
	ActionTrackDeleteSample(SampleRef *ref);
	~ActionTrackDeleteSample() override;

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	int index;
	SampleRef *ref;
};

#endif /* ACTIONTRACKDELETESAMPLE_H_ */
