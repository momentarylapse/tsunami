/*
 * ActionTrackAddSampleRef.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKADDSAMPLEREF_H_
#define ACTIONTRACKADDSAMPLEREF_H_

#include "../../Action.h"

class Sample;
class SampleRef;

class ActionTrackAddSampleRef: public Action
{
public:
	ActionTrackAddSampleRef(int track_no, int pos, int index);
	virtual ~ActionTrackAddSampleRef();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
	int pos;
};

#endif /* ACTIONTRACKADDSAMPLEREF_H_ */
