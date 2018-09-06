/*
 * TrackLayer.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_TRACKLAYER_H_
#define SRC_DATA_TRACKLAYER_H_

#include "Range.h"
#include "Midi/MidiData.h"
#include "Midi/Instrument.h"
#include "../Stuff/Observable.h"



class AudioBuffer;
class Song;
class SampleRef;
class Sample;
enum class SignalType;



class TrackLayer : public Observable<VirtualBase>
{
public:
	TrackLayer();
	TrackLayer(Track *track);
	~TrackLayer();

	Range range(int keep_notes = 0) const;
	AudioBuffer _cdecl _readBuffers(const Range &r, bool allow_ref);
	void _cdecl readBuffers(AudioBuffer &buf, const Range &r, bool allow_ref);
	void _cdecl read_buffers_fixed(AudioBuffer &buf, const Range &r);

	// actions
	AudioBuffer _cdecl _getBuffers(const Range &r);
	void _cdecl getBuffers(AudioBuffer &buf, const Range &r);

	void _cdecl insertMidiData(int offset, const MidiNoteBuffer &midi);
	void _cdecl addMidiNote(MidiNote *n);
	void _cdecl addMidiNotes(const MidiNoteBuffer &notes);
	void _cdecl deleteMidiNote(const MidiNote *note);

	SampleRef *_cdecl addSampleRef(int pos, Sample* sample);
	void _cdecl deleteSampleRef(SampleRef *ref);
	void _cdecl editSampleRef(SampleRef *ref, float volume, bool mute);

	void _cdecl make_own_track();

	void _cdecl mark_dominant(const Range &range);

	Track *track;
	Song *song() const;
	SignalType type;
	int channels;
	bool is_main();

	Array<AudioBuffer> buffers;

	MidiNoteBuffer midi;

	Array<SampleRef*> samples;

	int version_number() const;
};


#endif /* SRC_DATA_TRACKLAYER_H_ */
