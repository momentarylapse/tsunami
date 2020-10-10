/*
 * ActionSampleReplaceBuffer.cpp
 *
 *  Created on: 06.06.2019
 *      Author: michi
 */

#include "ActionSampleReplaceBuffer.h"
#include "../../Data/Sample.h"
#include <algorithm>

ActionSampleReplaceBuffer::ActionSampleReplaceBuffer(shared<Sample> s, AudioBuffer* _buf) {
	sample = s;
	buf = _buf;
}

void* ActionSampleReplaceBuffer::execute(Data* d) {
	std::swap(sample->buf, buf);
	sample->notify(sample->MESSAGE_CHANGE_BY_ACTION);
	return sample.get();
}

void ActionSampleReplaceBuffer::undo(Data* d) {
	execute(d);
}
