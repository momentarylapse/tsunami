/*
 * SampleRef.h
 *
 *  Created on: 29.05.2015
 *      Author: michi
 */

#ifndef SRC_DATA_SAMPLEREF_H_
#define SRC_DATA_SAMPLEREF_H_

#include "../Stuff/Observable.h"
#include "../lib/math/rect.h"

class Song;
class TrackLayer;
class Sample;
class MidiNoteBuffer;
class AudioBuffer;
class Range;
enum class SignalType;

class SampleRef : public Observable<VirtualBase>
{
public:
	SampleRef(Sample *sample);
	virtual ~SampleRef();
	void _cdecl __init__(Sample *sample);
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	Range _cdecl range() const;
	SignalType _cdecl type() const;

	int _cdecl get_index() const;

	int pos;
	Sample *origin;
	AudioBuffer *buf;
	MidiNoteBuffer *midi;
	bool muted;
	float volume;

	// editing
	rect area;
	TrackLayer *layer;
	Song *owner;
};

#endif /* SRC_DATA_SAMPLEREF_H_ */
