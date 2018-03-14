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

class Song;
class Track;
class SampleRef;
class TrackMarker;
class MidiNote;
class Bar;

class SongSelection
{
public:
	SongSelection();

	void clear();
	void clear_data();
	void all(Song *s);
	void _update_bars(Song *s);
	void from_range(Song *s, const Range &r, int mask = MASK_ALL);
	void from_range(Song *s, const Range &r, Set<const Track*> tracks, int mask = MASK_ALL);

	enum{
		MASK_SAMPLES = 1,
		MASK_MARKERS = 2,
		MASK_MIDI_NOTES = 4,
		MASK_BARS = 8,
		MASK_ALL = -1
	};

	Range range;

	Range bar_range;

	Set<const Track*> tracks;
	Set<const SampleRef*> samples;
	Set<const TrackMarker*> markers;
	Set<const MidiNote*> notes;
	Set<const Bar*> bars;
	Range bar_indices;

	void add(const Track *t);
	void set(const Track *t, bool selected);
	bool has(const Track *t) const;

	void add(const SampleRef *s);
	void set(const SampleRef *s, bool selected);
	bool has(const SampleRef *s) const;

	void add(const TrackMarker *m);
	void set(const TrackMarker *m, bool selected);
	bool has(const TrackMarker *m) const;

	void add(const MidiNote *n);
	void set(const MidiNote *n, bool selected);
	bool has(const MidiNote *n) const;

	void add(const Bar *b);
	void set(const Bar *b, bool selected);
	bool has(const Bar *b) const;

	int getNumSamples() const;

	SongSelection restrict_to_track(Track *t) const;
	SongSelection operator||(const SongSelection &s) const;
	SongSelection minus(const SongSelection &s) const;
};

#endif /* SRC_DATA_SONGSELECTION_H_ */
