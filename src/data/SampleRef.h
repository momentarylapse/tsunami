/*
 * SampleRef.h
 *
 *  Created on: 29.05.2015
 *      Author: michi
 */

#ifndef SRC_DATA_SAMPLEREF_H_
#define SRC_DATA_SAMPLEREF_H_

#include "../lib/pattern/Observable.h"
#include "../lib/base/pointer.h"
#include "../lib/math/rect.h"

class Song;
class TrackLayer;
class Sample;
class MidiNoteBuffer;
class AudioBuffer;
class Range;
enum class SignalType;

class SampleRef : public Sharable<obs::Node<VirtualBase>> {
public:
	SampleRef(shared<Sample> sample);
	virtual ~SampleRef();
	void _cdecl __init__(shared<Sample> sample);
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;
	obs::Source out_changed_by_action{this, "changed-by-action"};

	Range _cdecl range() const;
	SignalType _cdecl type() const;

	int _cdecl get_index() const;

	int pos;
	shared<Sample> origin;
	AudioBuffer &buf() const;
	MidiNoteBuffer &midi() const;
	bool muted;
	float volume;

	// editing
	rect area;
	TrackLayer *layer;
	Song *owner;
};

#endif /* SRC_DATA_SAMPLEREF_H_ */
