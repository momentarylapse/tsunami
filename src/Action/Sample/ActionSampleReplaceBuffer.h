/*
 * ActionSampleReplaceBuffer.h
 *
 *  Created on: 06.06.2019
 *      Author: michi
 */

#ifndef SRC_ACTION_SAMPLE_ACTIONSAMPLEREPLACEBUFFER_H_
#define SRC_ACTION_SAMPLE_ACTIONSAMPLEREPLACEBUFFER_H_

#include "../Action.h"

class AudioBuffer;
class Sample;

class ActionSampleReplaceBuffer : public Action
{
public:
	ActionSampleReplaceBuffer(Sample *s, AudioBuffer *buf);
	~ActionSampleReplaceBuffer() override;

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Sample *sample;
	AudioBuffer *buf;
};

#endif /* SRC_ACTION_SAMPLE_ACTIONSAMPLEREPLACEBUFFER_H_ */
