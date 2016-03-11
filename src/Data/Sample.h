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

class Song;
class Track;
class SampleRef;

class Sample : public Observable
{
public:
	Sample(int type);
	virtual ~Sample();

	static const string MESSAGE_CHANGE_BY_ACTION;
	static const string MESSAGE_REFERENCE;
	static const string MESSAGE_UNREFERENCE;

	int get_index();
	Range getRange();

	Song *owner;

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



#endif /* SAMPLE_H_ */
