/*
 * ActionSampleReplaceBuffer.h
 *
 *  Created on: 06.06.2019
 *      Author: michi
 */

#ifndef SRC_ACTION_SAMPLE_ACTIONSAMPLEREPLACEBUFFER_H_
#define SRC_ACTION_SAMPLE_ACTIONSAMPLEREPLACEBUFFER_H_

#include <lib/history/Action.h>

namespace tsunami {

struct AudioBuffer;
struct Sample;

class ActionSampleReplaceBuffer : public history::Action {
public:
	ActionSampleReplaceBuffer(shared<Sample> s, AudioBuffer *buf);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<Sample> sample;
	AudioBuffer *buf;
};

}

#endif /* SRC_ACTION_SAMPLE_ACTIONSAMPLEREPLACEBUFFER_H_ */
