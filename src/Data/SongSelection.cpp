/*
 * SongSelection.cpp
 *
 *  Created on: 06.03.2016
 *      Author: michi
 */

#include "SongSelection.h"
#include "Song.h"

SongSelection::SongSelection()
{
}

void SongSelection::clear()
{
	bars.clear();
	bar_range.clear();
	tracks.clear();
	clear_data();
}

void SongSelection::clear_data()
{
	samples.clear();
	markers.clear();
	notes.clear();
}

void SongSelection::all(Song* s)
{
	from_range(s, s->getRangeWithTime());
}

void SongSelection::from_range(Song *s, const Range &r)
{
	Set<const Track*> _tracks;
	for (Track *t: s->tracks)
		_tracks.add(t);
	from_range(s, r, _tracks);
}

void SongSelection::from_range(Song *s, const Range &r, Set<const Track*> _tracks)
{
	clear();
	range = r;

	for (const Track *t: s->tracks){
		if (_tracks.find(t) < 0)
			continue;
		add(t);

		// subs
		for (SampleRef *sr: t->samples)
			set(sr, range.overlaps(sr->range()));

		// markers
		for (TrackMarker *m: t->markers)
			set(m, range.overlaps(m->range));

		// midi
		for (MidiNote *n: t->midi)
			set(n, range.is_inside(n->range.center()));
	}
}

void SongSelection::update_bars(Song* s)
{
	bars.clear();
	bar_range.clear();

	int pos = 0;
	bool first = true;
	foreachi(BarPattern &b, s->bars, i){
		Range r = Range(pos + 1, b.length - 2);
		if (r.overlaps(range)){
			if (first){
				bars = Range(i, 1);
				bar_range = Range(pos, b.length);
			}
			bars.set_end(i+1);
			bar_range.set_end(r.end() + 1);
			first = false;
		}else if (range.length == 0 and (range.offset == pos)){
			bars = Range(i, 0);
			bar_range = Range(pos, 0);
		}
		pos += b.length;
	}
	if (range.start() >= pos){
		bars = Range(s->bars.num, 0);
		bar_range = Range(pos, 0);
	}
}

void SongSelection::add(const Track* t)
{
	tracks.add(t);
}

void SongSelection::set(const Track* t, bool selected)
{
	if (selected)
		tracks.add(t);
	else
		tracks.erase(t);
}

bool SongSelection::has(const Track* t) const
{
	return tracks.contains(t);
}

void SongSelection::add(const SampleRef* s)
{
	samples.add(s);
}

void SongSelection::set(const SampleRef* s, bool selected)
{
	if (selected)
		samples.add(s);
	else
		samples.erase(s);
}

bool SongSelection::has(const SampleRef* s) const
{
	return samples.contains(s);
}

void SongSelection::add(const TrackMarker* m)
{
	markers.add(m);
}

void SongSelection::set(const TrackMarker* m, bool selected)
{
	if (selected)
		markers.add(m);
	else
		markers.erase(m);
}

bool SongSelection::has(const TrackMarker* m) const
{
	return markers.contains(m);
}

void SongSelection::add(const MidiNote* n)
{
	notes.add(n);
}

void SongSelection::set(const MidiNote* n, bool selected)
{
	if (selected)
		notes.add(n);
	else
		notes.erase(n);
}

bool SongSelection::has(const MidiNote* n) const
{
	return notes.contains(n);
}

int SongSelection::getNumSamples() const
{
	return samples.num;
}

SongSelection SongSelection::restrict_to_track(Track *t) const
{
	SongSelection sel;
	sel.range = range;
	sel.bars = bars;
	sel.bar_range = bar_range;
	sel.set(t, true);
	for (auto n: t->midi)
		sel.set(n, has(n));
	for (auto m: t->markers)
		sel.set(m, has(m));
	for (auto r: t->samples)
		sel.set(r, has(r));
	return sel;
}

SongSelection SongSelection::operator||(const SongSelection &s) const
{
	SongSelection r = *this;
	for (auto t: s.tracks)
		r.add(t);
	for (auto n: s.notes)
		r.add(n);
	for (auto m: s.markers)
		r.add(m);
	for (auto sr: s.samples)
		r.add(sr);
	return r;
}

SongSelection SongSelection::minus(const SongSelection &s) const
{
	SongSelection r = *this;
	return r;
}
