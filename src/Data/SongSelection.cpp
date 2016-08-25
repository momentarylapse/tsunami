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
	samples.clear();
	markers.clear();
}

void SongSelection::all(Song* s)
{
	for (Track *t : s->tracks)
		add(t);
}


void SongSelection::fromRange(Song* s, const Range &r)
{
	range = r;

	// subs
	for (Track *t : s->tracks)
		for (SampleRef *s : t->samples)
			set(s, has(t) and range.overlaps(s->range()));
}

void SongSelection::update_bars(Song* s)
{

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
		}
		pos += b.length;
	}
}

void SongSelection::add(Track* t)
{
	tracks.add(t);
}

void SongSelection::set(Track* t, bool selected)
{
	if (selected)
		tracks.add(t);
	else
		tracks.erase(t);
}

bool SongSelection::has(Track* t) const
{
	return tracks.contains(t);
}

void SongSelection::add(SampleRef* s)
{
	samples.add(s);
}

void SongSelection::set(SampleRef* s, bool selected)
{
	if (selected)
		samples.add(s);
	else
		samples.erase(s);
}

bool SongSelection::has(SampleRef* s) const
{
	return samples.contains(s);
}

void SongSelection::add(TrackMarker* m)
{
	markers.add(m);
}

void SongSelection::set(TrackMarker* m, bool selected)
{
	if (selected)
		markers.add(m);
	else
		markers.erase(m);
}

bool SongSelection::has(TrackMarker* m) const
{
	return markers.contains(m);
}

int SongSelection::getNumSamples() const
{
	return samples.num;
}
