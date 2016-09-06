/*
 * MidiData.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "MidiData.h"
#include "Instrument.h"
#include "../Data/Track.h"
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
		return "C\u266F";
	if (pitch_rel == 2)
		return "D";
	if (pitch_rel == 3)
		return "D\u266F";
	if (pitch_rel == 4)
		return "E";
	if (pitch_rel == 5)
		return "F";
	if (pitch_rel == 6)
		return "F\u266F";
	if (pitch_rel == 7)
		return "G";
	if (pitch_rel == 8)
		return "G\u266F";
	if (pitch_rel == 9)
		return "A";
	if (pitch_rel == 10)
		return "A\u266F";
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
	if (pitch == DRUM_PITCH_BASS_ACCOUSTIC)
		return "bass      (akk)";
	if (pitch == DRUM_PITCH_BASS)
		return "bass";
	if (pitch == DRUM_PITCH_SIDE_STICK)
		return "side stick";
	if (pitch == DRUM_PITCH_SNARE)
		return "snare";
	if (pitch == DRUM_PITCH_CLAP)
		return "clap";
	if (pitch == DRUM_PITCH_SNARE_ELECTRONIC)
		return "snare     (electronic)";
	if (pitch == DRUM_PITCH_TOM_FLOOR_LOW)
		return "tom - floor low";
	if (pitch == DRUM_PITCH_HIHAT_CLOSED)
		return "hihat - closed";
	if (pitch == DRUM_PITCH_TOM_FLOOR_HI)
		return "tom - floor hi";
	if (pitch == DRUM_PITCH_HIHAT_PEDAL)
		return "hihat - pedal";
	if (pitch == DRUM_PITCH_TOM_LOW)
		return "tom - low";
	if (pitch == DRUM_PITCH_HIHAT_OPEN)
		return "hihat - open";
	if (pitch == DRUM_PITCH_TOM_LOW_MID)
		return "tom - low mid";
	if (pitch == DRUM_PITCH_TOM_HI_MID)
		return "tom - hi mid";
	if (pitch == DRUM_PITCH_CRASH_1)
		return "crash 1";
	if (pitch == DRUM_PITCH_TOM_HI)
		return "tom - hi";
	if (pitch == DRUM_PITCH_RIDE_1)
		return "ride 1";
	if (pitch == DRUM_PITCH_CHINESE)
		return "chinese";
	if (pitch == DRUM_PITCH_BELL_RIDE)
		return "bell ride";
	if (pitch == DRUM_PITCH_TAMBOURINE)
		return "tambourine";
	if (pitch == DRUM_PITCH_SPLASH)
		return "splash";
	if (pitch == DRUM_PITCH_COWBELL)
		return "cowbell";
	if (pitch == DRUM_PITCH_CRASH_2)
		return "crash 2";
	if (pitch == DRUM_PITCH_VIBRASLASH)
		return "vibraslash?";
	if (pitch == DRUM_PITCH_RIDE_2)
		return "ride 2";
	if (pitch == DRUM_PITCH_BONGO_HI)
		return "bongo - hi";
	if (pitch == DRUM_PITCH_BONGO_LOW)
		return "bongo - low";
	return pitch_name(pitch);
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

// needs to ignore the current this->samples... always set to r.length
int MidiRawData::read(MidiRawData &data, const Range &r) const
{
	data.samples = r.length;//min(r.length, samples - r.offset);
	for (MidiEvent &e : *this)
		if (r.is_inside(e.pos))
			data.add(MidiEvent(e.pos - r.offset, e.pitch, e.volume));
	return data.samples;
}

Array<MidiNote> MidiRawData::getNotes(const Range &r) const
{
	MidiData a = midi_events_to_notes(*this);
	Array<MidiNote> b;
	for (MidiNote &n : a)
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
	for (int p : active)
		add(MidiEvent(r.end(), p, 0));
}

void MidiRawData::addMetronomeClick(int pos, int level, float volume)
{
	if (level == 0){
		add(MidiEvent(pos, 81, volume));
		add(MidiEvent(pos, 81, 0));
	}else if (level == 1){
		add(MidiEvent(pos, 74, volume * 0.5f));
		add(MidiEvent(pos, 74, 0));
	}else{
		add(MidiEvent(pos, 66, volume * 0.1f));
		add(MidiEvent(pos, 66, 0));
	}
}

void MidiRawData::append(const MidiRawData &data)
{
	for (MidiEvent &e : data)
		add(MidiEvent(e.pos + samples, e.pitch, e.volume));
	samples += data.samples;
}

MidiData::MidiData()
{
	samples = 0;
}

void MidiData::__init__()
{
	new(this) MidiData;
}

MidiRawData MidiData::getEvents(const Range &r) const
{
	MidiDataRef b = getNotes(r);
	return midi_notes_to_events(b);
}

MidiDataRef MidiData::getNotes(const Range &r) const
{
	MidiData b;
	int first = -1;
	int last = -1;
	foreachi(MidiNote &n, *this, i)
		if (r.overlaps(n.range)){
			first = i;
			break;
		}
	foreachib(MidiNote &n, *this, i)
		if (r.overlaps(n.range)){
			last = i;
			break;
		}
	if (first >= 0)
		b.set_ref(sub(first, last - first + 1));
	return b;
}

MidiDataRef MidiData::getNotesSafe(const Range &r) const
{
	MidiData b;
	int first = -1;
	int last = -1;
	foreachi(MidiNote &n, *this, i)
		if (r.is_inside(n.range.center())){
			first = i;
			break;
		}
	foreachib(MidiNote &n, *this, i)
		if (r.is_inside(n.range.center())){
			last = i;
			break;
		}
	if (first >= 0)
		b.set_ref(sub(first, last - first + 1));
	return b;
}

Range MidiData::getRange(int elongation) const
{
	if (num == 0)
		return Range::EMPTY;
	int i0 = (*this)[0].range.offset;
	int i1 = back().range.end(); // FIXME...
	return Range(i0, i1 - i0 + elongation);
}

void MidiData::sort()
{
	for (int i=0;i<num;i++)
		for (int j=i+1;j<num;j++)
			if ((*this)[i].range.offset > (*this)[j].range.offset)
				swap(i, j);
}

void MidiData::sanify(const Range &r)
{
	sort();
}

MidiDataRef::MidiDataRef(const MidiData &midi)
{
	set_ref(midi);
}

MidiRawData midi_notes_to_events(const MidiData &notes)
{
	MidiRawData r;
	for (MidiNote &n : notes){
		r.add(MidiEvent(n.range.offset, n.pitch, n.volume));
		r.add(MidiEvent(n.range.end()-1, n.pitch, 0));
	}
	r.samples = notes.samples;
	return r;
}

MidiData midi_events_to_notes(const MidiRawData &events)
{
	MidiData a;
	MidiRawData b;
	for (MidiEvent &e : events){
		if (e.volume > 0){
			bool exists = false;
			for (MidiEvent &bb : b)
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
	a.samples = events.samples;
	return a;
}


string chord_type_name(int type)
{
	if (type == CHORD_TYPE_MINOR)
		return _("Minor");
	if (type == CHORD_TYPE_MAJOR)
		return _("Major");
	if (type == CHORD_TYPE_DIMINISHED)
		return _("Diminished");
	if (type == CHORD_TYPE_AUGMENTED)
		return _("Augmented");
	return "???";
}

Array<int> chord_notes(int type, int inversion, int pitch)
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

struct HandPosition
{
	HandPosition()
	{
		offset = 0;
		value = 0;
	}
	HandPosition(int _offset, int _value)
	{
		offset = _offset;
		value = _value;
	}
	bool operator>(const HandPosition &o) const
	{ return offset > o.offset; }
	bool operator==(const HandPosition &o) const
	{ return offset == o.offset; }
	int offset, value;
};


void MidiData::update_meta(Track *t, const Scale& scale) const
{
	Set<HandPosition> hands;
	for (TrackMarker &m : t->markers)
		if (m.text.match(":pos *:"))
			hands.add(HandPosition(m.pos, m.text.substr(4, -2)._int()));


	int hand_position = 0;
	int next = 0;

	for (MidiNote &n : *this){
		while(next < hands.num){
			if (n.range.offset >= hands[next].offset)
				hand_position = hands[next ++].value;
			else
				break;
		}

		n.update_meta(t->instrument, scale, hand_position);
	}
}

void MidiData::clear_meta() const
{
	for (MidiNote &n : *this)
		n.stringno = -1;
	for (MidiNote &n : *this){
		n.clef_position = -1;
		n.modifier = MODIFIER_UNKNOWN;
	}
}
