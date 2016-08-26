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

class SongSelection
{
public:
	SongSelection();

	void clear();
	void all(Song *s);
	void all_tracks(Song *s);
	void fromRange(Song *s, const Range &r);
	void update_bars(Song *s);

	Range range;

	Range bars;
	Range bar_range;

	Set<Track*> tracks;
	Set<SampleRef*> samples;
	Set<TrackMarker*> markers;
	Set<MidiNote*> notes;

	void add(Track *t);
	void set(Track *t, bool selected);
	bool has(Track *t) const;

	void add(SampleRef *s);
	void set(SampleRef *s, bool selected);
	bool has(SampleRef *s) const;

	void add(TrackMarker *m);
	void set(TrackMarker *m, bool selected);
	bool has(TrackMarker *m) const;

	void add(MidiNote *n);
	void set(MidiNote *n, bool selected);
	bool has(MidiNote *n) const;

	int getNumSamples() const;

	SongSelection restrict_to_track(Track *t) const;
};

#endif /* SRC_DATA_SONGSELECTION_H_ */
