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
#include "TrackMarker.h"
#include "SampleRef.h"

SongSelection::SongSelection() {
}

void SongSelection::clear() {
	layers.clear();
	clear_data();
}

void SongSelection::clear_data() {
	samples.clear();
	markers.clear();
	notes.clear();
	bars.clear();
}

SongSelection SongSelection::all(Song* s) {
	return from_range(s, Range::ALL);
}

SongSelection SongSelection::from_range(Song *song, const Range &r) {
	SongSelection s;
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();

	s._update_bars(song);

	for (Track *t: song->tracks) {

		for (auto *m: t->markers)
			s.set(m, s.range.overlaps(m->range));
	}

	for (Track *t: song->tracks) {
		for (auto *l: t->layers) {
			s.add(l);

			for (auto *n: l->midi)
				s.set(n, s.range.overlaps(n->range));

			for (auto *sr: l->samples)
				s.set(sr, s.range.overlaps(sr->range()));
		}
	}

	return s;
}

SongSelection SongSelection::filter(const Array<const TrackLayer*> &_layers) const {
	SongSelection s;
	s.range = range;

	for (auto *l: _layers) {
		if (has(l))
			s.add(l);

		for (auto m: l->track->markers)
			if (has(m))
				s.add(m);

		for (auto *n: l->midi)
			if (has(n))
				s.add(n);

		for (auto *sr: l->samples)
			if (has(sr))
				s.add(sr);

		if (l->type == SignalType::BEATS)
			s.bars = bars;
	}

	return s;
}

SongSelection SongSelection::filter(int mask) const {
	auto s = *this;

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

void SongSelection::_update_bars(Song* s) {
	bars.clear();


	if (range.end() <= 0)
		return;

	int pos = 0;
	foreachi(Bar *b, s->bars, i) {
		Range r = Range(pos + 1, b->length - 2);
		b->offset = pos;
		if (r.overlaps(range))
			bars.add(b);
		pos += b->length;
	}
}

bool SongSelection::has(const Track *t) const {
	for (auto *l: t->layers)
		if (has(l))
			return true;
	return false;
}

Set<const Track*> SongSelection::tracks() const {
	Set<const Track*> tracks;
	for (auto *l: layers)
		tracks.add(l->track);
	return tracks;
}


#define IMPLEMENT_FUNC(TYPE, ARRAY)                    \
void SongSelection::add(const TYPE* t)                 \
{ ARRAY.add(t); }                                      \
void SongSelection::set(const TYPE* t, bool selected)  \
{ if (selected) ARRAY.add(t); else ARRAY.erase(t); }   \
bool SongSelection::has(const TYPE* t) const           \
{ return ARRAY.contains(t); }                          \
void SongSelection::toggle(const TYPE* t)              \
{ set(t, !has(t)); }                                   \
void SongSelection::click(const TYPE* t, bool control) \
{ if (control){ toggle(t); }else{ if (!has(t)){ clear_data(); add(t); } } }



IMPLEMENT_FUNC(TrackLayer, layers)
IMPLEMENT_FUNC(SampleRef, samples)
IMPLEMENT_FUNC(TrackMarker, markers)
IMPLEMENT_FUNC(MidiNote, notes)
IMPLEMENT_FUNC(Bar, bars)


int SongSelection::num_samples() const {
	return samples.num;
}

bool SongSelection::is_empty() const {
	if (!range.empty())
		return false;
	return (samples.num == 0) and (markers.num == 0) and (notes.num == 0) and (bars.num == 0);
}

Array<int> SongSelection::bar_indices(Song *song) const {
	Array<int> indices;
	foreachi(Bar *b, song->bars, i)
		if (has(b))
			indices.add(i);
	return indices;
}

template<class T>
Array<const T*> constify_array(const Array<T*> &array) {
	Array<const T*> r;
	for (auto *x: array)
		r.add(x);
	return r;
}

SongSelection SongSelection::restrict_to_track(Track *t) const {
	return filter(constify_array(t->layers));
}

SongSelection SongSelection::restrict_to_layer(TrackLayer *l) const {
	return filter({l});
}

SongSelection SongSelection::operator||(const SongSelection &s) const {
	SongSelection r = *this;
	for (auto l: s.layers)
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

SongSelection SongSelection::operator&&(const SongSelection &s) const {
	SongSelection r;
	for (auto l: s.layers)
		if (has(l))
			r.add(l);
	for (auto n: s.notes)
		if (has(n))
			r.add(n);
	for (auto m: s.markers)
		if (has(m))
			r.add(m);
	for (auto sr: s.samples)
		if (has(sr))
			r.add(sr);
	for (auto b: s.bars)
		if (has(b))
			r.add(b);
	return r;
}

SongSelection SongSelection::minus(const SongSelection &s) const {
	SongSelection r = *this;
	for (auto l: s.layers)
		r.set(l, false);
	for (auto n: s.notes)
		r.set(n, false);
	for (auto m: s.markers)
		r.set(m, false);
	for (auto sr: s.samples)
		r.set(sr, false);
	for (auto b: s.bars)
		r.set(b, false);
	return r;
}
