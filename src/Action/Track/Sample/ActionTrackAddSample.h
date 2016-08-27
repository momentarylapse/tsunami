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
class Track;

class ActionTrackAddSample: public Action
{
public:
	ActionTrackAddSample(Track *t, int pos, Sample* sample);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	Sample* sample;
	int pos;
};

#endif /* ACTIONTRACKADDSAMPLE_H_ */
