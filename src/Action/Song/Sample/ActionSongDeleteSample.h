/*
 * ActionSongDeleteSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSONGDELETESAMPLE_H_
#define ACTIONSONGDELETESAMPLE_H_

#include "../../Action.h"

class Sample;

class ActionSongDeleteSample : public Action
{
public:
	ActionSongDeleteSample(Sample *s);
	virtual ~ActionSongDeleteSample();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Sample *sample;
	int index;
};

#endif /* ACTIONSONGDELETESAMPLE_H_ */
