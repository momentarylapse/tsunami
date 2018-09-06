/*
 * SongSelection.cpp
 *
 *  Created on: 06.03.2016
 *      Author: michi
 */

#include "SongSelection.h"

#include "Rhythm/Bar.h"
#include "base.h"
#include "Song.h"
#include "Track.h"
#include "TrackLayer.h"
#include "SampleRef.h"

SongSelection::SongSelection()
{
	bar_gap = -1;
}

void SongSelection::clear()
{
	bar_indices.clear();
	tracks.clear();
	track_layers.clear();
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
	from_range(s, s->range_with_time());
}

SongSelection SongSelection::from_range(Song *song, const Range &r)
{
	Set<const Track*> _tracks;
	for (Track *t: song->tracks)
		_tracks.add(t);
	Set<const TrackLayer*> _layers;
	for (Track *t: song->tracks)
		for (TrackLayer *l: t->layers)
			_layers.add(l);
	return from_range(song, r, _tracks, _layers);
}

SongSelection SongSelection::from_range(Song *song, const Range &r, Set<const Track*> _tracks, Set<const TrackLayer*> _layers)
{
	SongSelection s;
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();


	for (const Track *t: song->tracks){
		if (!_tracks.contains(t))
			continue;
		s.add(t);

		// markers
		if (_layers.contains(t->layers[0]))
		for (TrackMarker *m: t->markers)
			s.set(m, s.range.overlaps(m->range));
	}

	for (const Track *t: song->tracks){
		for (const TrackLayer *l: t->layers){
			if (!_layers.contains(l))
				continue;
			s.add(l);

			// midi
			for (MidiNote *n: l->midi)
				s.set(n, s.range.overlaps(n->range));

			// samples
			for (SampleRef *sr: l->samples)
				s.set(sr, s.range.overlaps(sr->range()));
		}
	}

	// bars
	s._update_bars(song);
	return s;
}

SongSelection SongSelection::filter(int mask) const
{
	SongSelection s = *this;

	if ((mask & Mask::SAMPLES) == 0)
		s.samples.clear();
	if ((mask & Mask::MIDI_NOTES) == 0)
		s.notes.clear();
	if ((mask & Mask::BARS) == 0)
		s.bars.clear();
	if ((mask & Mask::MARKERS) == 0)
		s.markers.clear();

	return s;
}

void SongSelection::_update_bars(Song* s)
{
	bars.clear();
	bar_indices.clear();
	bar_gap = -1;

	bool metro_selected = false;
	for (Track *t: s->tracks)
		if (has(t) and t->type == SignalType::BEATS)
			metro_selected = true;
	if (!metro_selected)
		return;


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


// make sure a track is selected iff 1+ layers are selected
void SongSelection::_update_tracks_from_layers(Song *s)
{
	for (Track *t: s->tracks){
		bool any_layer_selected = false;
		for (TrackLayer *l: t->layers)
			any_layer_selected |= has(l);
		set(t, any_layer_selected);
	}
}

void SongSelection::make_consistent(Song *s)
{
	_update_tracks_from_layers(s);
}


#define IMPLEMENT_FUNC(TYPE, ARRAY)                   \
void SongSelection::add(const TYPE* t)                \
{ ARRAY.add(t); }                                     \
void SongSelection::set(const TYPE* t, bool selected) \
{ if (selected) ARRAY.add(t); else ARRAY.erase(t); }  \
bool SongSelection::has(const TYPE* t) const          \
{ return ARRAY.contains(t); }                         \
void SongSelection::click(const TYPE* t, bool control)  \
{ if (control){ set(t, !has(t)); }else{ if (!has(t)){ clear_data(); add(t); } } }



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
	for (auto *l: t->layers){
		sel.set(l, true);
		for (auto n: l->midi)
			sel.set(n, has(n));
		for (auto r: l->samples)
			sel.set(r, has(r));
	}
	for (auto m: t->markers)
		sel.set(m, has(m));
	return sel;
}

SongSelection SongSelection::operator||(const SongSelection &s) const
{
	SongSelection r = *this;
	for (auto t: s.tracks)
		r.add(t);
	for (auto l: s.track_layers)
		r.add(l);
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
