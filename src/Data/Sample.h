/*
 * Sample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLE_H_
#define SAMPLE_H_

#include "BufferBox.h"
#include "../lib/math/rect.h"

class AudioFile;
class Track;

class Sample
{
public:
	Sample();
	~Sample();

	AudioFile *owner;

	string name;
	BufferBox buf;
	float volume;
	int offset;

	int ref_count;
	bool auto_delete;

	void ref();
	void unref();
};



class SampleRef
{
public:
	SampleRef(Sample *sample);
	~SampleRef();

	Track *GetParent();
	Range GetRange();

	int pos;
	Sample *origin;
	BufferBox &buf;
	bool muted;
	float volume;

	// repetition
	int rep_num;
	int rep_delay;

	// editing
	rect area;
	int track_no;
	AudioFile *owner;

	bool is_selected;
};

#endif /* SAMPLE_H_ */
