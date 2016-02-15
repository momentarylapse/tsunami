/*
 * MidiData.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "MidiData.h"
#include <math.h>
#include "../lib/hui/hui.h"


float pitch_to_freq(float pitch)
{
	return 440.0f * pow(2, (pitch - 69.0f) / 12.0f);
}

float freq_to_pitch(float freq)
{
	return log2(freq / 440.0f) * 12.0f + 69.0f;
}


// "scientific" notation
//   naive MIDI octave is off by 1
int pitch_get_octave(int pitch)
{
	return (pitch / 12) - 1;
}

int pitch_from_octave_and_rel(int rel, int octave)
{
	return rel + octave * 12 + 12;
}

int pitch_to_rel(int pitch)
{
	return pitch % 12;
}

string rel_pitch_name(int pitch_rel)
{
	if (pitch_rel == 0)
		return "C";
	if (pitch_rel == 1)
		return "C#";
	if (pitch_rel == 2)
		return "D";
	if (pitch_rel == 3)
		return "D#";
	if (pitch_rel == 4)
		return "E";
	if (pitch_rel == 5)
		return "F";
	if (pitch_rel == 6)
		return "F#";
	if (pitch_rel == 7)
		return "G";
	if (pitch_rel == 8)
		return "G#";
	if (pitch_rel == 9)
		return "A";
	if (pitch_rel == 10)
		return "A#";
	if (pitch_rel == 11)
		return "B";
	return "???";
}

// convert an integer to a string
string i2s_small(int i)
{
	string r;
	int l=0;
	bool m=false;
	if (i<0){
		i=-i;
		m=true;
	}
	char a[128];
	while (1){
		a[l]=(i%10)+0x80;
		a[l+1]=0x82;
		a[l+2]=0xe2;
		l+=3;
		i=(int)(i/10);
		if (i==0)
			break;
	}
	if (m){
		a[l]=0x8b;
		a[l+1]=0x82;
		a[l+2]=0xe2;
		l+=3;
	}
	r.resize(l);
	for (int j=0;j<l;j++)
		r[l-j-1]=a[j];
	return r;
}

string pitch_name(int pitch)
{
	return rel_pitch_name(pitch_to_rel(pitch)) + i2s_small(pitch_get_octave(pitch));
}

string drum_pitch_name(int pitch)
{
	if (pitch == 35)	return "bass      (akk)";
	if (pitch == 36)	return "bass";
	if (pitch == 37)	return "side stick";
	if (pitch == 38)	return "snare";
	if (pitch == 39)	return "clap";
	if (pitch == 40)	return "snare     (electronic)";
	if (pitch == 41)	return "tom - floor low";
	if (pitch == 42)	return "hihat - closed";
	if (pitch == 43)	return "tom - floor hi";
	if (pitch == 44)	return "hihat - pedal";
	if (pitch == 45)	return "tom - low";
	if (pitch == 46)	return "hihat - open";
	if (pitch == 47)	return "tom - low mid";
	if (pitch == 48)	return "tom - hi mid";
	if (pitch == 49)	return "crash 1";
	if (pitch == 50)	return "tom - hi";
	if (pitch == 51)	return "ride 1";
	if (pitch == 52)	return "chinese";
	if (pitch == 53)	return "bell ride";
	if (pitch == 54)	return "tambourine";
	if (pitch == 55)	return "splash";
	if (pitch == 56)	return "cowbell";
	if (pitch == 57)	return "crash 2";
	if (pitch == 58)	return "vibraslash?";
	if (pitch == 59)	return "ride 2";
	if (pitch == 60)	return "bongo - hi";
	if (pitch == 61)	return "bongo - low";
	return pitch_name(pitch);
}

MidiNote::MidiNote(const Range &_range, float _pitch, float _volume)
{
	range = _range;
	pitch = _pitch;
	volume = _volume;
}

float MidiNote::getFrequency()
{
	return 440.0f * pow(2.0f, (float)(pitch - 69) / 12.0f);
}

MidiRawData::MidiRawData()
{
	samples = 0;
}

void MidiRawData::__init__()
{
	new(this) MidiRawData;
}

MidiRawData MidiRawData::getEvents(const Range &r) const
{
	MidiRawData a;
	for (int i=0;i<num;i++)
		if (r.is_inside((*this)[i].pos))
			a.add((*this)[i]);
	return a;
}

int MidiRawData::read(MidiRawData &data, const Range &r) const
{
	data.samples = min(r.num, samples - r.offset);
	foreach(MidiEvent &e, const_cast<MidiRawData&>(*this))
		if (r.is_inside(e.pos))
			data.add(MidiEvent(e.pos - r.offset, e.pitch, e.volume));
	return data.samples;
}

Array<MidiNote> MidiRawData::getNotes(const Range &r) const
{
	MidiNoteData a = midi_events_to_notes(*this);
	Array<MidiNote> b;
	foreach(MidiNote &n, a)
		if (r.overlaps(n.range))
			b.add(n);
	return b;
}

int MidiRawData::getNextEvent(int pos) const
{
	return 0;
}

Range MidiRawData::getRange(int elongation) const
{
	if (num == 0)
		return Range::EMPTY;
	int i0 = (*this)[0].pos;
	int i1 = back().pos;
	return Range(i0, i1 - i0 + elongation);
}

void MidiRawData::sort()
{
	for (int i=0;i<num;i++)
		for (int j=i+1;j<num;j++)
			if ((*this)[i].pos > (*this)[j].pos)
				swap(i, j);
}

void MidiRawData::sanify(const Range &r)
{
	//int max_pos = 0;
	Set<int> active;
	Array<int> del_me;

	sort();

	// analyze
	foreachi(MidiEvent &e, *this, i){
		int p = e.pitch;

		// out of range
		if (!r.is_inside(e.pos)){
			del_me.add(i);
			continue;
		}

		if (e.volume > 0){
			active.add(p);
		}else{
			if (active.contains(p)){
				active.erase(p);
			}else{
				// unnecessary stop -> delete
				del_me.add(i);
			}
		}
	}

	// delete
	foreachb(int i, del_me)
		erase(i);

	// wrong end mark?
//	if (max_pos > samples)
//		samples = max_pos;

	// end active notes
	foreach(int p, active)
		add(MidiEvent(r.end(), p, 0));
}

void MidiRawData::addMetronomeClick(int pos, int level, float volume)
{
	if (level == 0){
		add(MidiEvent(pos, 81, volume));
		add(MidiEvent(pos+1, 81, 0));
	}else{
		add(MidiEvent(pos, 74, volume * 0.5f));
		add(MidiEvent(pos+1, 74, 0));
	}
}

void MidiRawData::append(const MidiRawData &data)
{
	foreach(MidiEvent &e, const_cast<MidiRawData&>(data))
		add(MidiEvent(e.pos + samples, e.pitch, e.volume));
	samples += data.samples;
}

MidiNoteData::MidiNoteData()
{
	samples = 0;
}

void MidiNoteData::__init__()
{
	new(this) MidiNoteData;
}

MidiRawData MidiNoteData::getEvents(const Range &r) const
{
	MidiNoteData b = getNotes(r);
	return midi_notes_to_events(b);
}

MidiNoteData MidiNoteData::getNotes(const Range &r) const
{
	MidiNoteData b;
	foreach(MidiNote &n, const_cast<MidiNoteData&>(*this))
		if (r.overlaps(n.range))
			b.add(n);
	return b;
}

MidiNoteData MidiNoteData::getNotesSafe(const Range &r) const
{
	MidiNoteData b;
	foreach(MidiNote &n, const_cast<MidiNoteData&>(*this))
		if (r.is_inside(n.range.center()))
			b.add(n);
	return b;
}

Range MidiNoteData::getRange(int elongation) const
{
	if (num == 0)
		return Range::EMPTY;
	int i0 = (*this)[0].range.offset;
	int i1 = back().range.end(); // FIXME...
	return Range(i0, i1 - i0 + elongation);
}

void MidiNoteData::sort()
{
	for (int i=0;i<num;i++)
		for (int j=i+1;j<num;j++)
			if ((*this)[i].range.offset > (*this)[j].range.offset)
				swap(i, j);
}

void MidiNoteData::sanify(const Range &r)
{
	sort();
}

MidiEvent::MidiEvent(int _pos, float _pitch, float _volume)
{
	pos = _pos;
	pitch = _pitch;
	volume = _volume;
}

MidiRawData midi_notes_to_events(const MidiNoteData &notes)
{
	MidiRawData r;
	foreach(MidiNote &n, const_cast<MidiNoteData&>(notes)){
		r.add(MidiEvent(n.range.offset, n.pitch, n.volume));
		r.add(MidiEvent(n.range.end()-1, n.pitch, 0));
	}
	return r;
}

MidiNoteData midi_events_to_notes(const MidiRawData &events)
{
	MidiNoteData a;
	MidiRawData b;
	foreach(MidiEvent &e, const_cast<MidiRawData&>(events)){
		if (e.volume > 0){
			bool exists = false;
			foreach(MidiEvent &bb, b)
				if ((int)bb.pitch == (int)e.pitch){
					exists = true;
					break;
				}
			if (!exists)
				b.add(e);
		}else{
			foreachi(MidiEvent &bb, b, i)
				if ((int)bb.pitch == (int)e.pitch){
					MidiNote n = MidiNote(Range(bb.pos, e.pos - bb.pos), bb.pitch, bb.volume);
					a.add(n);
					b.erase(i);
					break;
				}
		}
	}
	return a;
}


string GetChordTypeName(int type)
{
	if (type == CHORD_TYPE_MINOR)
		return _("Moll");
	if (type == CHORD_TYPE_MAJOR)
		return _("Dur");
	if (type == CHORD_TYPE_DIMINISHED)
		return _("Vermindert");
	if (type == CHORD_TYPE_AUGMENTED)
		return _("&Uberm&a&sig");
	return "???";
}

Array<string> GetChordTypeNames()
{
	Array<string> r;
	for (int i=0; i<NUM_CHORD_TYPES; i++)
		r.add(GetChordTypeName(i));
	return r;
}

Array<int> GetChordNotes(int type, int inversion, int pitch)
{
	Array<int> r;
	r.add(pitch);
	if (type == CHORD_TYPE_MINOR){
		r.add(pitch + 3);
		r.add(pitch + 7);
	}else if (type == CHORD_TYPE_MAJOR){
		r.add(pitch + 4);
		r.add(pitch + 7);
	}else if (type == CHORD_TYPE_DIMINISHED){
		r.add(pitch + 3);
		r.add(pitch + 6);
	}else if (type == CHORD_TYPE_AUGMENTED){
		r.add(pitch + 4);
		r.add(pitch + 8);
	}
	if (inversion == 2)
		r.insert(r.pop() - 12, 0);
	if (inversion == 1){
		r.insert(r.pop() - 12, 0);
		r.insert(r.pop() - 12, 0);
	}
	return r;
}

