/*
 * ActionSampleScale.cpp
 *
 *  Created on: 22.04.2017
 *      Author: michi
 */

#include "ActionSampleScale.h"

#include "../../Data/Song.h"
#include "../../Data/BufferInterpolator.h"

ActionSampleScale::ActionSampleScale(Sample *s, int _new_size, int _method)
{
	sample = s;
	new_size = _new_size;
	method = _method;
	buf = NULL;
}

ActionSampleScale::~ActionSampleScale()
{
	if (buf)
		delete buf;
}

void *ActionSampleScale::execute(Data *d)
{
	//Song *a = dynamic_cast<Song*>(d);

	if (!buf){
		buf = new BufferBox;
		BufferInterpolator::interpolate(sample->buf, *buf, new_size, (BufferInterpolator::Method)method);
	}

	sample->buf.swap_ref(*buf);

	sample->notify(sample->MESSAGE_CHANGE);
	return sample;
}

void ActionSampleScale::undo(Data *d)
{
	//Song *a = dynamic_cast<Song*>(d);

	sample->buf.swap_ref(*buf);

	sample->notify(sample->MESSAGE_CHANGE);
}

