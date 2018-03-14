/*
 * SongSelection.cpp
 *
 *  Created on: 06.03.2016
 *      Author: michi
 */

#include "SongSelection.h"
#include "Song.h"
#include "../Rhythm/Bar.h"

SongSelection::SongSelection()
{
}

void SongSelection::clear()
{
	bar_indices.clear();
	bar_range.clear();
	tracks.clear();
	clear_data();
}

void SongSelection::clear_data()
{
	samples.clear();
	markers.clear();
	notes.clear();
	bars.clear();
}

void SongSelection::all(Song* s)
{
	from_range(s, s->getRangeWithTime());
}

void SongSelection::from_range(Song *s, const Range &r, int mask)
{
	Set<const Track*> _tracks;
	for (Track *t: s->tracks)
		_tracks.add(t);
	from_range(s, r, _tracks, mask);
}

void SongSelection::from_range(Song *s, const Range &r, Set<const Track*> _tracks, int mask)
{
	clear();
	range = r;

	for (const Track *t: s->tracks){
		if (_tracks.find(t) < 0)
			continue;
		add(t);

		// samples
		if ((mask & MASK_SAMPLES) > 0)
			for (SampleRef *sr: t->samples)
				set(sr, range.overlaps(sr->range()));

		// markers
		if ((mask & MASK_MARKERS) > 0)
			for (TrackMarker *m: t->markers)
				set(m, range.overlaps(m->range));

		// midi
		if ((mask & MASK_MIDI_NOTES) > 0)
			for (MidiNote *n: t->midi)
				set(n, range.is_inside(n->range.center()));
	}

	// bars
	if ((mask & MASK_BARS) > 0)
		_update_bars(s);
}

void SongSelection::_update_bars(Song* s)
{
	bars.clear();
	bar_indices.clear();
	bar_range.clear();

	int pos = 0;
	bool first = true;
	foreachi(Bar *b, s->bars, i){
		Range r = Range(pos + 1, b->length - 2);
		b->offset = pos;
		if (r.overlaps(range)){
			if (first){
				bar_indices = Range(i, 1);
				bar_range = Range(pos, b->length);
			}
			bar_indices.set_end(i + 1);
			bars.add(b);
			bar_range.set_end(r.end() + 1);
			first = false;
		}else if (range.length == 0 and (range.offset == pos)){
			bar_indices = Range(i, 0);
			bars.add(b);
			bar_range = Range(pos, 0);
		}
		pos += b->length;
	}
	if (range.start() >= pos){
		bar_indices = Range(s->bars.num, 0);
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

void SongSelection::add(const Bar *b)
{
	bars.add(b);
}

void SongSelection::set(const Bar *b, bool selected)
{
	if (selected)
		bars.add(b);
	else
		bars.erase(b);
}

bool SongSelection::has(const Bar *b) const
{
	return bars.contains(b);
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
	for (auto b: s.bars)
		r.add(b);
	return r;
}

SongSelection SongSelection::minus(const SongSelection &s) const
{
	SongSelection r = *this;
	return r;
}
