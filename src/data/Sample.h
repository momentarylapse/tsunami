/*
 * Sample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SRC_DATA_SAMPLE_H_
#define SRC_DATA_SAMPLE_H_

#include "midi/MidiData.h"
#include "../lib/base/pointer.h"
#include "../lib/pattern/Observable.h"
#include "audio/AudioBuffer.h"

namespace tsunami {

class Song;
class Track;
class SampleRef;
class Tag;
enum class SignalType;

class Sample : public Sharable<obs::Node<VirtualBase>> {
public:
	explicit Sample(SignalType type);
	Sample(const string &name, const AudioBuffer &buf);
	Sample(const string &name, const MidiNoteBuffer &buf);
	~Sample() override;

	void __init__(const string &name, const AudioBuffer &buf);

	obs::source out_changed_by_action{this, "changed-by-action"};
	obs::source out_reference{this, "reference"};
	obs::source out_unreference{this, "unreference"};

	int get_index() const;
	Range _cdecl range() const;

	Song *owner;
	void set_owner(Song *s);
	void unset_owner();

	string name;
	SignalType type;
	AudioBuffer *buf;
	MidiNoteBuffer midi;
	float volume;
	int offset;

	int uid;
	int ref_count;
	bool auto_delete;

	Array<Tag> tags;
	string _cdecl get_value(const string &key) const;
	void _cdecl set_value(const string &key, const string &value);

	void _cdecl ref();
	void _cdecl unref();

	xfer<SampleRef> _cdecl create_ref();

	static int create_uid();
};

}


#endif /* SRC_DATA_SAMPLE_H_ */
