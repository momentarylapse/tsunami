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
class Track;
class CrossFade;
enum class SignalType;



class TrackLayer : public Observable<VirtualBase> {
public:
	TrackLayer();
	TrackLayer(Track *track);
	~TrackLayer();

	Range range(int keep_notes = 0) const;
	AudioBuffer _cdecl _read_buffers(const Range &r, bool allow_ref);
	void _cdecl read_buffers(AudioBuffer &buf, const Range &r, bool allow_ref);
	void _cdecl read_buffers_fixed(AudioBuffer &buf, const Range &r);

	// actions
	AudioBuffer _cdecl _get_buffers(const Range &r);
	void _cdecl get_buffers(AudioBuffer &buf, const Range &r);

	void _cdecl insert_midi_data(int offset, const MidiNoteBuffer &midi);
	void _cdecl add_midi_note(MidiNote *n);
	void _cdecl add_midi_notes(const MidiNoteBuffer &notes);
	void _cdecl edit_midi_note(MidiNote *n, const Range &range, float pitch, float volume);
	void _cdecl midi_note_set_string(MidiNote *n, int stringno);
	void _cdecl midi_note_set_flags(MidiNote *n, int flags);
	void _cdecl delete_midi_note(const MidiNote *note);

	SampleRef *_cdecl add_sample_ref(int pos, Sample* sample);
	void _cdecl delete_sample_ref(SampleRef *ref);
	void _cdecl edit_sample_ref(SampleRef *ref, float volume, bool mute);

	void _cdecl make_own_track();

	void _cdecl mark_dominant(const Range &range);
	void _cdecl mark_add_dominant(const Range &range);

	Track *track;
	Song *song() const;
	SignalType type;
	int channels;
	bool is_main() const;

	Array<AudioBuffer> buffers;

	MidiNoteBuffer midi;

	Array<SampleRef*> samples;

	Array<CrossFade> fades;

	int version_number() const;

	Array<Range> active_version_ranges() const;
	Array<Range> inactive_version_ranges() const;
};


#endif /* SRC_DATA_TRACKLAYER_H_ */
