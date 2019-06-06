/*
 * ActionSampleReplaceBuffer.cpp
 *
 *  Created on: 06.06.2019
 *      Author: michi
 */

#include "ActionSampleReplaceBuffer.h"
#include "../../Data/Sample.h"
#include <algorithm>

ActionSampleReplaceBuffer::ActionSampleReplaceBuffer(Sample* s, AudioBuffer* _buf) {
	sample = s;
	buf = _buf;
}

ActionSampleReplaceBuffer::~ActionSampleReplaceBuffer() {
	delete buf;
}

void* ActionSampleReplaceBuffer::execute(Data* d) {
	std::swap(sample->buf, buf);
	return sample;
}

void ActionSampleReplaceBuffer::undo(Data* d) {
	execute(d);
}
