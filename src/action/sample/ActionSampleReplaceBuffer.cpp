/*
 * ActionSampleReplaceBuffer.cpp
 *
 *  Created on: 06.06.2019
 *      Author: michi
 */

#include "ActionSampleReplaceBuffer.h"
#include "../../data/Sample.h"
#include <algorithm>

namespace tsunami {

ActionSampleReplaceBuffer::ActionSampleReplaceBuffer(shared<Sample> s, AudioBuffer* _buf) {
	sample = s;
	buf = _buf;
}

void* ActionSampleReplaceBuffer::execute(Data* d) {
	std::swap(sample->buf, buf);
	sample->out_changed_by_action.notify();
	return sample.get();
}

void ActionSampleReplaceBuffer::undo(Data* d) {
	execute(d);
}

}
