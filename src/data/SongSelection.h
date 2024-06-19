/*
 * SongSelection.h
 *
 *  Created on: 06.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_SONGSELECTION_H_
#define SRC_DATA_SONGSELECTION_H_

#include "../lib/base/base.h"
#include "../lib/base/set.h"
#include "Range.h"

namespace tsunami {

class Song;
class Track;
class TrackLayer;
class SampleRef;
class TrackMarker;
class MidiNote;
class Bar;

class SongSelection {
public:
	SongSelection();

	void clear();
	void clear_data();
	void _update_bars(Song *s);
	static SongSelection all(Song *s);
	static SongSelection from_range(Song *s, const Range &r);

	enum Mask {
		SAMPLES = 1,
		MARKERS = 2,
		MIDI_NOTES = 4,
		BARS = 8,
		ALL = -1
	};
	SongSelection filter(int mask) const;
	SongSelection filter(const Array<const TrackLayer*> &layers) const;

	Song *song;
	Range range_raw;
	Range range() const;

	// data, might contain illegal pointers!
	base::set<const TrackLayer*> _layers;
	base::set<const SampleRef*> _samples;
	base::set<const TrackMarker*> _markers;
	base::set<const MidiNote*> _notes;
	base::set<const Bar*> _bars;

	base::set<const Track*> tracks() const;
	base::set<const TrackLayer*> layers() const;

	bool has(const Track *t) const;

	void add(const TrackLayer *l);
	void set(const TrackLayer *l, bool selected);
	bool has(const TrackLayer *l) const;
	void toggle(const TrackLayer *l);
	void click(const TrackLayer *l, bool control_pressed);

	void add(const SampleRef *s);
	void set(const SampleRef *s, bool selected);
	bool has(const SampleRef *s) const;
	void toggle(const SampleRef *s);
	void click(const SampleRef *s, bool control_pressed);

	void add(const TrackMarker *m);
	void set(const TrackMarker *m, bool selected);
	bool has(const TrackMarker *m) const;
	void toggle(const TrackMarker *m);
	void click(const TrackMarker *m, bool control_pressed);

	void add(const MidiNote *n);
	void set(const MidiNote *n, bool selected);
	bool has(const MidiNote *n) const;
	void toggle(const MidiNote *n);
	void click(const MidiNote *n, bool control_pressed);

	void add(const Bar *b);
	void set(const Bar *b, bool selected);
	bool has(const Bar *b) const;
	void toggle(const Bar *b);
	void click(const Bar *b, bool control_pressed);

	int num_samples() const;
	bool is_empty() const;
	Array<int> bar_indices(Song *song) const;

	SongSelection restrict_to_track(Track *t) const;
	SongSelection restrict_to_layer(TrackLayer *l) const;
	SongSelection operator||(const SongSelection &s) const;
	SongSelection operator&&(const SongSelection &s) const;
	SongSelection minus(const SongSelection &s) const;
};

}

#endif /* SRC_DATA_SONGSELECTION_H_ */
