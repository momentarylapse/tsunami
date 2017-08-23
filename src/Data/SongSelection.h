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
	void update_bars(Song *s);

	Range range;

	Range bars;
	Range bar_range;

	Set<const Track*> tracks;
	Set<const SampleRef*> samples;
	Set<const TrackMarker*> markers;
	Set<const MidiNote*> notes;

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

	int getNumSamples() const;

	SongSelection restrict_to_track(Track *t) const;
};

#endif /* SRC_DATA_SONGSELECTION_H_ */
