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
	song = nullptr;
}

void SongSelection::clear() {
	_layers.clear();
	clear_data();
}

void SongSelection::clear_data() {
	_samples.clear();
	_markers.clear();
	_notes.clear();
	_bars.clear();
}

Range SongSelection::range() const {
	return range_raw.canonical();
}

SongSelection SongSelection::all(Song* s) {
	return from_range(s, Range::ALL);
}

SongSelection SongSelection::from_range(Song *song, const Range &r) {
	SongSelection s;
	s.song = song;
	s.range_raw = r;

	s._update_bars(song);

	for (Track *t: weak(song->tracks)) {
		for (auto *l: weak(t->layers)) {
			s.add(l);

			for (auto *n: weak(l->midi))
				s.set(n, s.range().overlaps(n->range));
				
			for (auto *m: weak(l->markers))
				s.set(m, s.range().overlaps(m->range));

			for (auto *sr: weak(l->samples))
				s.set(sr, s.range().overlaps(sr->range()));
		}
	}

	return s;
}

bool layer_legal(Song *s, const TrackLayer *l) {
	if (!s)
		return true;
	return s->layers().find((TrackLayer*)l) >= 0;
}

// assumes, _layers to be legal pointers
SongSelection SongSelection::filter(const Array<const TrackLayer*> &_layers) const {
	SongSelection s;
	s.song = song;
	s.range_raw = range_raw;

	for (auto *l: _layers) {

		if (has(l))
			s.add(l);

		for (auto m: weak(l->markers))
			if (has(m))
				s.add(m);

		for (auto *n: weak(l->midi))
			if (has(n))
				s.add(n);

		for (auto *sr: weak(l->samples))
			if (has(sr))
				s.add(sr);

		if (l->type == SignalType::BEATS)
			s._bars = _bars;
	}

	return s;
}

SongSelection SongSelection::filter(int mask) const {
	auto s = *this;

	if ((mask & Mask::SAMPLES) == 0)
		s._samples.clear();
	if ((mask & Mask::MIDI_NOTES) == 0)
		s._notes.clear();
	if ((mask & Mask::BARS) == 0)
		s._bars.clear();
	if ((mask & Mask::MARKERS) == 0)
		s._markers.clear();

	return s;
}

void SongSelection::_update_bars(Song* s) {
	_bars.clear();


	if (range().end() <= 0)
		return;

	int pos = 0;
	foreachi(Bar *b, weak(s->bars), i) {
		Range r = Range(pos + 1, b->length - 2);
		b->offset = pos;
		if (r.overlaps(range()))
			_bars.add(b);
		pos += b->length;
	}
}

bool SongSelection::has(const Track *t) const {
	for (auto *l: weak(t->layers))
		if (has(l))
			return true;
	return false;
}

Set<const Track*> SongSelection::tracks() const {
	Set<const Track*> tracks;
	for (auto *l: layers())
		tracks.add(l->track);
	return tracks;
}

Set<const TrackLayer*> SongSelection::layers() const {
	Set<const TrackLayer*> layers;
	for (auto *l: _layers)
		if (layer_legal(song, l))
			layers.add(l);
	return layers;
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



IMPLEMENT_FUNC(TrackLayer, _layers)
IMPLEMENT_FUNC(SampleRef, _samples)
IMPLEMENT_FUNC(TrackMarker, _markers)
IMPLEMENT_FUNC(MidiNote, _notes)
IMPLEMENT_FUNC(Bar, _bars)


int SongSelection::num_samples() const {
	return _samples.num;
}

bool SongSelection::is_empty() const {
	if (!range().is_empty())
		return false;
	return (_samples.num == 0) and (_markers.num == 0) and (_notes.num == 0) and (_bars.num == 0);
}

Array<int> SongSelection::bar_indices(Song *song) const {
	Array<int> indices;
	foreachi(Bar *b, weak(song->bars), i)
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
	return filter(constify_array(weak(t->layers)));
}

SongSelection SongSelection::restrict_to_layer(TrackLayer *l) const {
	return filter({l});
}

SongSelection SongSelection::operator||(const SongSelection &s) const {
	SongSelection r = *this;
	for (auto l: s._layers)
		r.add(l);
	for (auto n: s._notes)
		r.add(n);
	for (auto m: s._markers)
		r.add(m);
	for (auto sr: s._samples)
		r.add(sr);
	for (auto b: s._bars)
		r.add(b);
	return r;
}

SongSelection SongSelection::operator&&(const SongSelection &s) const {
	SongSelection r;
	r.song = song;
	for (auto l: s._layers)
		if (has(l))
			r.add(l);
	for (auto n: s._notes)
		if (has(n))
			r.add(n);
	for (auto m: s._markers)
		if (has(m))
			r.add(m);
	for (auto sr: s._samples)
		if (has(sr))
			r.add(sr);
	for (auto b: s._bars)
		if (has(b))
			r.add(b);
	return r;
}

SongSelection SongSelection::minus(const SongSelection &s) const {
	SongSelection r = *this;
	for (auto l: s._layers)
		r.set(l, false);
	for (auto n: s._notes)
		r.set(n, false);
	for (auto m: s._markers)
		r.set(m, false);
	for (auto sr: s._samples)
		r.set(sr, false);
	for (auto b: s._bars)
		r.set(b, false);
	return r;
}
