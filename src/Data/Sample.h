/*
 * Sample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLE_H_
#define SAMPLE_H_

#include "BufferBox.h"
#include "MidiData.h"
#include "../lib/math/rect.h"
#include "../Stuff/Observable.h"

class AudioFile;
class Track;
class SampleRef;

class Sample
{
public:
	Sample(int type);
	~Sample();

	int get_index();

	AudioFile *owner;

	string name;
	int type;
	BufferBox buf;
	MidiData midi;
	float volume;
	int offset;

	int uid;
	int ref_count;
	bool auto_delete;

	void ref();
	void unref();

	SampleRef *create_ref();
};



class SampleRef : public Observable
{
public:
	SampleRef(Sample *sample);
	virtual ~SampleRef();
	void _cdecl __init__(Sample *sample);
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	Track *GetParent();
	Range GetRange();

	int get_index();

	int pos;
	Sample *origin;
	BufferBox *buf;
	MidiData *midi;
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
