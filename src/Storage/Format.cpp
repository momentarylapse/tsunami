/*
 * Format.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Format.h"

Format::Format(const string &_extension, int _flags)
{
	extension = _extension;
	flags = _flags;
}

Format::~Format()
{
}

void Format::ImportData(Track *t, void *data, int channels, int bits, int samples, int offset)
{
	msg_db_r("ImportData", 1);

	BufferBox buf = t->GetBuffers(Range(offset, samples));
	buf.import(data, channels, bits, samples);

	msg_db_l(1);
}


bool Format::CanHandle(const string & _extension)
{
	return (extension == _extension);
}

