/*
 * MidiNoteBuffer.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "MidiData.h"
#include "Instrument.h"
#include "../SongSelection.h"
#include "../../lib/hui/hui.h"
#include <math.h>


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
	if (pitch == DrumPitch::BASS_ACCOUSTIC)
		return "bass      (akk)";
	if (pitch == DrumPitch::BASS)
		return "bass";
	if (pitch == DrumPitch::SIDE_STICK)
		return "side stick";
	if (pitch == DrumPitch::SNARE)
		return "snare";
	if (pitch == DrumPitch::CLAP)
		return "clap";
	if (pitch == DrumPitch::SNARE_ELECTRONIC)
		return "snare     (electronic)";
	if (pitch == DrumPitch::TOM_FLOOR_LOW)
		return "tom - floor low";
	if (pitch == DrumPitch::HIHAT_CLOSED)
		return "hihat - closed";
	if (pitch == DrumPitch::TOM_FLOOR_HI)
		return "tom - floor hi";
	if (pitch == DrumPitch::HIHAT_PEDAL)
		return "hihat - pedal";
	if (pitch == DrumPitch::TOM_LOW)
		return "tom - low";
	if (pitch == DrumPitch::HIHAT_OPEN)
		return "hihat - open";
	if (pitch == DrumPitch::TOM_LOW_MID)
		return "tom - low mid";
	if (pitch == DrumPitch::TOM_HI_MID)
		return "tom - hi mid";
	if (pitch == DrumPitch::CRASH_1)
		return "crash 1";
	if (pitch == DrumPitch::TOM_HI)
		return "tom - hi";
	if (pitch == DrumPitch::RIDE_1)
		return "ride 1";
	if (pitch == DrumPitch::CHINESE)
		return "chinese";
	if (pitch == DrumPitch::BELL_RIDE)
		return "bell ride";
	if (pitch == DrumPitch::TAMBOURINE)
		return "tambourine";
	if (pitch == DrumPitch::SPLASH)
		return "splash";
	if (pitch == DrumPitch::COWBELL)
		return "cowbell";
	if (pitch == DrumPitch::CRASH_2)
		return "crash 2";
	if (pitch == DrumPitch::VIBRASLASH)
		return "vibraslash?";
	if (pitch == DrumPitch::RIDE_2)
		return "ride 2";
	if (pitch == DrumPitch::BONGO_HI)
		return "bongo - hi";
	if (pitch == DrumPitch::BONGO_LOW)
		return "bongo - low";
	return pitch_name(pitch);
}

string modifier_symbol(NoteModifier mod)
{
	if (mod == NoteModifier::NONE)
		return "";
	if (mod == NoteModifier::SHARP)
		return "\u266f";
	if (mod == NoteModifier::FLAT)
		return "\u266d";
	if (mod == NoteModifier::NATURAL)
		return "\u266e";
	return "?";
}

int modifier_apply(int pitch, NoteModifier mod)
{
	return pitch + modifier_shift(mod);
}

int modifier_apply(int pitch, NoteModifier mod, NoteModifier scale_mod)
{
	return pitch + modifier_shift(combine_note_modifiers(mod, scale_mod));
}

int modifier_shift(NoteModifier mod)
{
	if (mod == NoteModifier::SHARP)
		return 1;
	if (mod == NoteModifier::FLAT)
		return -1;
	return 0;
}

NoteModifier combine_note_modifiers(NoteModifier mod, NoteModifier scale_mod)
{
	if (mod == NoteModifier::NATURAL)
		return NoteModifier::NONE;
	int shift = modifier_shift(mod) + modifier_shift(scale_mod);
	if (shift > 0)
		return NoteModifier::SHARP;
	if (shift < 0)
		return NoteModifier::FLAT;
	return NoteModifier::NONE;
}


MidiEventBuffer::MidiEventBuffer()
{
	samples = 0;
}

void MidiEventBuffer::__init__()
{
	new(this) MidiEventBuffer;
}

void MidiEventBuffer::clear()
{
	Array<MidiEvent>::clear();
	samples = 0;
}

MidiEventBuffer MidiEventBuffer::get_events(const Range &r) const
{
	MidiEventBuffer a;
	for (int i=0;i<num;i++)
		if (r.is_inside((*this)[i].pos))
			a.add((*this)[i]);
	return a;
}

// needs to ignore the current this->samples... always set to r.length
int MidiEventBuffer::read(MidiEventBuffer &data, const Range &r) const
{
	data.samples = r.length;//min(r.length, samples - r.offset);
	for (MidiEvent &e: *this)
		if (r.is_inside(e.pos))
			data.add(MidiEvent(e.pos - r.offset, e.pitch, e.volume));
	return data.samples;
}

Array<MidiNote> MidiEventBuffer::get_notes(const Range &r) const
{
	MidiNoteBuffer a = midi_events_to_notes(*this);
	Array<MidiNote> b;
	for (MidiNote *n: a)
		if (r.overlaps(n->range))
			b.add(*n);
	return b;
}

int MidiEventBuffer::get_next_event(int pos) const
{
	return 0;
}

Range MidiEventBuffer::range(int elongation) const
{
	if (num == 0)
		return Range::EMPTY;
	int i0 = (*this)[0].pos;
	int i1 = back().pos;
	return Range(i0, i1 - i0 + elongation);
}

void MidiEventBuffer::sort()
{
	for (int i=0;i<num;i++)
		for (int j=i+1;j<num;j++)
			if ((*this)[i].pos > (*this)[j].pos)
				swap(i, j);
}

void MidiEventBuffer::sanify(const Range &r)
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
	for (int p: active)
		add(MidiEvent(r.end(), p, 0));
}

void MidiEventBuffer::add_note(const Range &r, float pitch, float volume)
{
	add(MidiEvent(r.start(), pitch, volume));
	add(MidiEvent(r.end(), pitch, 0));
}

void MidiEventBuffer::add_metronome_click(int pos, int level, float volume)
{
	if (level == 0){
		add(MidiEvent(pos, 81, volume));
		add(MidiEvent(pos, 81, 0));
	}else if (level == 1){
		add(MidiEvent(pos, 74, volume * 0.7f));
		add(MidiEvent(pos, 74, 0));
	}else{
		add(MidiEvent(pos, 71, volume * 0.5f));
		add(MidiEvent(pos, 71, 0));
	}
}

void MidiEventBuffer::append(const MidiEventBuffer &data)
{
	for (MidiEvent &e: data)
		add(MidiEvent(e.pos + samples, e.pitch, e.volume));
	samples += data.samples;
}

MidiNoteBuffer::MidiNoteBuffer()
{
	samples = 0;
}

MidiNoteBuffer::MidiNoteBuffer(const MidiNoteBuffer &midi)
{
	*this = midi;
}

MidiNoteBuffer::~MidiNoteBuffer()
{
	deep_clear();
}

void MidiNoteBuffer::__init__()
{
	new(this) MidiNoteBuffer;
}

void MidiNoteBuffer::__delete__()
{
	this->MidiNoteBuffer::~MidiNoteBuffer();
}

void MidiNoteBuffer::clear()
{
	Array<MidiNote*>::clear();
	samples = 0;
}

void MidiNoteBuffer::deep_clear()
{
	for (MidiNote *n: *this)
		delete(n);
	clear();
}

MidiEventBuffer MidiNoteBuffer::get_events(const Range &r) const
{
	MidiNoteBufferRef b = get_notes(r);
	return midi_notes_to_events(b);
}

MidiNoteBufferRef MidiNoteBuffer::get_notes(const Range &r) const
{
	MidiNoteBufferRef b;
	for (MidiNote *n: *this)
		if (r.overlaps(n->range))
			b.add(n);
	return b;
}

MidiNoteBufferRef MidiNoteBuffer::get_notes_by_selection(const SongSelection &s) const
{
	MidiNoteBufferRef b;
	for (MidiNote *n: *this)
		if (s.has(n))
			b.add(n);
	return b;
}

MidiNoteBuffer MidiNoteBuffer::duplicate() const
{
	MidiNoteBuffer b = *this;
	return b;
}

void MidiNoteBuffer::append(const MidiNoteBuffer &midi, int offset)
{
	for (MidiNote *n: midi){
		MidiNote *nn = n->copy();
		nn->range.offset += offset;
		add(nn);
	}
	samples = max(samples, midi.samples + offset);
	sort();
}

void MidiNoteBuffer::operator=(const MidiNoteBuffer &midi)
{
	deep_clear();

	for (MidiNote *n: midi)
		add(n->copy());

	samples = midi.samples;
}

// deep copy!
void MidiNoteBuffer::operator=(const MidiNoteBufferRef &midi)
{
	deep_clear();

	for (MidiNote *n: midi)
		add(n->copy());

	samples = midi.samples;
}

Range MidiNoteBuffer::range(int elongation) const
{
	if (num == 0)
		return Range::EMPTY;
	int i0 = (*this)[0]->range.offset;
	int i1 = back()->range.end(); // FIXME...
	return RangeTo(i0, i1 + elongation);
}

void MidiNoteBuffer::sort()
{
	for (int i=0;i<num;i++)
		for (int j=i+1;j<num;j++)
			if ((*this)[i]->range.offset > (*this)[j]->range.offset)
				swap(i, j);
}

void MidiNoteBuffer::sanify(const Range &r)
{
	sort();
}

MidiNoteBufferRef::MidiNoteBufferRef()
{
}

MidiNoteBufferRef::~MidiNoteBufferRef()
{
	clear();
}

MidiEventBuffer midi_notes_to_events(const MidiNoteBuffer &notes)
{
	MidiEventBuffer r;
	for (MidiNote *n: notes){
		Range rr = n->range;
		if (n->is(NOTE_FLAG_STACCATO))
			rr = Range(rr.offset, rr.length/2);
		/*if (n->is(NOTE_FLAG_TRILL)){
			int l = rr.length;
			r.add(MidiEvent(n));
			r.add(MidiEvent(rr.offset + l/4, n->pitch, 0));
			r.add(MidiEvent(rr.offset + l/4, n->pitch+1, n->volume));
			r.add(MidiEvent(rr.offset + l/2, n->pitch+1, 0));
			r.add(MidiEvent(rr.offset + l/2, n->pitch, n->volume));
			r.add(MidiEvent(rr.end()-1, n->pitch, 0));
		}else*/{
			r.add(MidiEvent(n));
			r.add(MidiEvent(rr.end()-1, n->pitch, 0));
		}
	}
	r.samples = notes.samples;
	return r;
}

MidiNoteBuffer midi_events_to_notes(const MidiEventBuffer &events)
{
	MidiNoteBuffer a;
	MidiEventBuffer start_events;
	for (MidiEvent &e: events){
		if (e.volume > 0){
			bool exists = false;
			for (MidiEvent &bb: start_events)
				if ((int)bb.pitch == (int)e.pitch){
					exists = true;
					break;
				}
			if (!exists)
				start_events.add(e);
		}else{
			foreachi(MidiEvent &bb, start_events, i)
				if ((int)bb.pitch == (int)e.pitch){
					MidiNote *n = new MidiNote(RangeTo(bb.pos, e.pos), bb.pitch, bb.volume);
					n->flags = bb.flags;
					n->stringno = bb.stringno;
					n->clef_position = bb.clef_position;
					a.add(n);
					start_events.erase(i);
					break;
				}
		}
	}
	a.samples = events.samples;
	return a;
}


string chord_type_name(ChordType type)
{
	if (type == ChordType::MINOR)
		return _("Minor");
	if (type == ChordType::MAJOR)
		return _("Major");
	if (type == ChordType::DIMINISHED)
		return _("Diminished");
	if (type == ChordType::AUGMENTED)
		return _("Augmented");
	return "???";
}

Array<int> chord_notes(ChordType type, int inversion, int pitch)
{
	Array<int> r;
	r.add(pitch);
	if (type == ChordType::MINOR){
		r.add(pitch + 3);
		r.add(pitch + 7);
	}else if (type == ChordType::MAJOR){
		r.add(pitch + 4);
		r.add(pitch + 7);
	}else if (type == ChordType::DIMINISHED){
		r.add(pitch + 3);
		r.add(pitch + 6);
	}else if (type == ChordType::AUGMENTED){
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


void MidiNoteBuffer::update_meta(const Instrument &instrument, const Scale& scale) const
{
	for (MidiNote *n: *this)
		n->update_meta(instrument, scale);
}

void MidiNoteBuffer::clear_meta() const
{
	for (MidiNote *n: *this)
		n->stringno = -1;
	for (MidiNote *n: *this){
		n->clef_position = -1;
		n->modifier = NoteModifier::UNKNOWN;
	}
}
