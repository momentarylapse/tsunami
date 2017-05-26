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
class Track;

class ActionTrackDeleteSample : public Action
{
public:
	ActionTrackDeleteSample(SampleRef *ref);
	virtual ~ActionTrackDeleteSample();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no, index;
	SampleRef *ref;
};

#endif /* ACTIONTRACKDELETESAMPLE_H_ */
