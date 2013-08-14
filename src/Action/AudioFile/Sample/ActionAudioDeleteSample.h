/*
 * ActionAudioDeleteSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONAUDIODELETESAMPLE_H_
#define ACTIONAUDIODELETESAMPLE_H_

#include "../../Action.h"

class Sample;

class ActionAudioDeleteSample : public Action
{
public:
	ActionAudioDeleteSample(int index);
	virtual ~ActionAudioDeleteSample();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Sample *sample;
	int index;
};

#endif /* ACTIONAUDIODELETESAMPLE_H_ */
