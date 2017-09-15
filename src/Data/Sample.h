/*
 * Sample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLE_H_
#define SAMPLE_H_

#include "../Midi/MidiData.h"
#include "../lib/math/rect.h"
#include "../Stuff/Observable.h"
#include "AudioBuffer.h"

class Song;
class Track;
class SampleRef;
struct Tag;

class Sample : public Observable<VirtualBase>
{
public:
	Sample(int type);
	virtual ~Sample();

	static const string MESSAGE_CHANGE_BY_ACTION;
	static const string MESSAGE_REFERENCE;
	static const string MESSAGE_UNREFERENCE;

	int get_index() const;
	Range _cdecl range() const;

	Song *owner;
	void set_owner(Song *s);
	void unset_owner();

	string name;
	int type;
	AudioBuffer buf;
	MidiData midi;
	float volume;
	int offset;

	int uid;
	int ref_count;
	bool auto_delete;

	int _pointer_ref_count;
	Sample *_pointer_ref();
	void _pointer_unref();

	Array<Tag> tags;
	string _cdecl getValue(const string &key) const;

	void _cdecl ref();
	void _cdecl unref();

	SampleRef *_cdecl create_ref();
};



#endif /* SAMPLE_H_ */
