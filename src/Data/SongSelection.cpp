/*
 * SongSelection.cpp
 *
 *  Created on: 06.03.2016
 *      Author: michi
 */

#include "SongSelection.h"

#include "Rhythm/Bar.h"
#include "Song.h"

SongSelection::SongSelection()
{
	bar_gap = -1;
}

void SongSelection::clear()
{
	bar_indices.clear();
	tracks.clear();
	clear_data();
}

void SongSelection::clear_data()
{
	samples.clear();
	markers.clear();
	notes.clear();
	bars.clear();
	bar_gap = -1;
}

void SongSelection::all(Song* s)
{
	from_range(s, s->getRangeWithTime());
}

SongSelection SongSelection::from_range(Song *song, const Range &r, int mask)
{
	Set<const Track*> _tracks;
	for (Track *t: song->tracks)
		_tracks.add(t);
	return from_range(song, r, _tracks, mask);
}

SongSelection SongSelection::from_range(Song *song, const Range &r, Set<const Track*> _tracks, int mask)
{
	SongSelection s;
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();


	for (const Track *t: song->tracks){
		if (_tracks.find(t) < 0)
			continue;
		s.add(t);

		// samples
		if ((mask & Mask::SAMPLES) > 0)
			for (SampleRef *sr: t->samples)
				s.set(sr, s.range.overlaps(sr->range()));

		// markers
		if ((mask & Mask::MARKERS) > 0)
			for (TrackMarker *m: t->markers)
				s.set(m, s.range.overlaps(m->range));

		// midi
		if ((mask & Mask::MIDI_NOTES) > 0)
			for (MidiNote *n: t->midi)
				//set(n, range.is_inside(n->range.center()));
				s.set(n, s.range.overlaps(n->range));
	}

	// bars
	if ((mask & Mask::BARS) > 0)
		s._update_bars(song);
	return s;
}

void SongSelection::_update_bars(Song* s)
{
	bars.clear();
	bar_indices.clear();
	bar_gap = -1;

	if (range.end() <= 0){
		bar_gap = 0;
		return;
	}

	int pos = 0;
	bool first = true;
	foreachi(Bar *b, s->bars, i){
		Range r = Range(pos + 1, b->length - 2);
		b->offset = pos;
		if (r.overlaps(range)){
			if (first){
				bar_indices = Range(i, 1);
			}
			bar_indices.set_end(i + 1);
			bars.add(b);
			first = false;
		}else if (range.length == 0 and (range.offset == pos)){
			bar_gap = i;
		}
		pos += b->length;
	}
	if (range.start() >= pos){
		bar_gap = s->bars.num;
	}
}



#define IMPLEMENT_FUNC(TYPE, ARRAY)                   \
void SongSelection::add(const TYPE* t)                \
{ ARRAY.add(t); }                                     \
void SongSelection::set(const TYPE* t, bool selected) \
{ if (selected) ARRAY.add(t); else ARRAY.erase(t); }  \
bool SongSelection::has(const TYPE* t) const          \
{ return ARRAY.contains(t); }


IMPLEMENT_FUNC(Track, tracks)
IMPLEMENT_FUNC(TrackLayer, track_layers)
IMPLEMENT_FUNC(SampleRef, samples)
IMPLEMENT_FUNC(TrackMarker, markers)
IMPLEMENT_FUNC(MidiNote, notes)
IMPLEMENT_FUNC(Bar, bars)


int SongSelection::num_samples() const
{
	return samples.num;
}

bool SongSelection::is_empty() const
{
	if (!range.empty())
		return false;
	return (samples.num == 0) and (markers.num == 0) and (notes.num == 0) and (bars.num == 0);
}

SongSelection SongSelection::restrict_to_track(Track *t) const
{
	SongSelection sel;
	sel.range = range;
	sel.bars = bars;
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
