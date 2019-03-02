/*
 * Sample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SRC_DATA_SAMPLE_H_
#define SRC_DATA_SAMPLE_H_

#include "Midi/MidiData.h"
#include "../lib/math/rect.h"
#include "../Stuff/Observable.h"
#include "Audio/AudioBuffer.h"

class Song;
class Track;
class SampleRef;
struct Tag;
enum class SignalType;

class Sample : public Observable<VirtualBase>
{
public:
	Sample(SignalType type);
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
	SignalType type;
	AudioBuffer buf;
	MidiNoteBuffer midi;
	float volume;
	int offset;

	int uid;
	int ref_count;
	bool auto_delete;

	int _pointer_ref_count;
	Sample *_pointer_ref();
	void _pointer_unref();

	Array<Tag> tags;
	string _cdecl get_value(const string &key) const;
	void _cdecl set_value(const string &key, const string &value);

	void _cdecl ref();
	void _cdecl unref();

	SampleRef *_cdecl create_ref();
};



#endif /* SRC_DATA_SAMPLE_H_ */
