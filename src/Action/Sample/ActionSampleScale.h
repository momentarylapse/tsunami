/*
 * ActionSampleScale.h
 *
 *  Created on: 22.04.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_SAMPLE_ACTIONSAMPLESCALE_H_
#define SRC_ACTION_SAMPLE_ACTIONSAMPLESCALE_H_


#include "../Action.h"

class BufferBox;
class Sample;

class ActionSampleScale : public Action
{
public:
	ActionSampleScale(Sample *s, int new_size, int method);
	virtual ~ActionSampleScale();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Sample *sample;
	BufferBox *buf;
	int new_size;
	int method;
};

#endif /* SRC_ACTION_SAMPLE_ACTIONSAMPLESCALE_H_ */
