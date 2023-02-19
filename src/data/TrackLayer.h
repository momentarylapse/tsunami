/*
 * TrackLayer.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_TRACKLAYER_H_
#define SRC_DATA_TRACKLAYER_H_

#include "Range.h"
#include "midi/MidiData.h"
#include "midi/Instrument.h"
#include "../lib/base/pointer.h"
#include "../lib/base/set.h"
#include "../lib/pattern/Observable.h"



class AudioBuffer;
class Song;
class SampleRef;
class Sample;
class Track;
class CrossFade;
class TrackMarker;
class Action;
enum class SignalType;



class TrackLayer : public Sharable<Observable<VirtualBase>> {
public:
	TrackLayer();
	TrackLayer(Track *track);
	~TrackLayer();

	Range range(int keep_notes = 0) const;
	AudioBuffer _cdecl _read_buffers(const Range &r, bool allow_ref);
	void _cdecl read_buffers(AudioBuffer &buf, const Range &r, bool allow_ref);
	void _cdecl read_buffers_fixed(AudioBuffer &buf, const Range &r);

	// actions
	void _cdecl get_buffers(AudioBuffer &buf, const Range &r);
	Action _cdecl *edit_buffers(AudioBuffer &buf, const Range &r);
	void _cdecl edit_buffers_finish(Action *a);

	void _cdecl insert_midi_data(int offset, const MidiNoteBuffer &midi);
	void _cdecl add_midi_note(shared<MidiNote> n);
	void _cdecl add_midi_notes(const MidiNoteBuffer &notes);
	void _cdecl edit_midi_note(MidiNote *n, const Range &range, float pitch, float volume);
	void _cdecl midi_note_set_string(MidiNote *n, int stringno);
	void _cdecl midi_note_set_flags(MidiNote *n, int flags);
	void _cdecl delete_midi_note(const MidiNote *note);

	shared<SampleRef> _cdecl add_sample_ref(int pos, Sample* sample);
	void _cdecl delete_sample_ref(SampleRef *ref);
	void _cdecl edit_sample_ref(SampleRef *ref, float volume, bool mute);
	
	const shared<TrackMarker> _cdecl add_marker(const shared<TrackMarker> marker);
	void _cdecl delete_marker(const TrackMarker *marker);
	void _cdecl edit_marker(const TrackMarker *marker, const Range &range, const string &text);

	void _cdecl make_own_track();

	void _cdecl version_activate(const Range &range, bool activate);
	void _cdecl set_muted(bool muted);

	Track *track;
	Song *song() const;
	SignalType type;
	int channels;
	bool muted;
	bool is_main() const;

	Array<AudioBuffer> buffers;

	MidiNoteBuffer midi;

	shared_array<SampleRef> samples;
	
	shared_array<TrackMarker> markers;
	Array<TrackMarker*> markers_sorted() const;

	Array<CrossFade> fades;

	int version_number() const;

	Array<Range> active_version_ranges() const;
	Array<Range> inactive_version_ranges() const;
};


base::set<const TrackLayer*> layer_set(const Array<TrackLayer*> &layers);


#endif /* SRC_DATA_TRACKLAYER_H_ */
