/*
 * MidiNote.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_MIDINOTE_H_
#define SRC_DATA_MIDI_MIDINOTE_H_

#include "../../lib/base/pointer.h"
#include "../Range.h"

class Scale;
class Clef;
class Instrument;
enum class NoteModifier;

enum {
	NOTE_FLAG_TRILL = 1<<0,
	NOTE_FLAG_LEGATO = 1<<1,
	NOTE_FLAG_STACCATO = 1<<2,
	NOTE_FLAG_TENUTO = 1<<3,
	NOTE_FLAG_HAMMER_ON = 1<<4,
	NOTE_FLAG_PULL_OFF = 1<<5,
	NOTE_FLAG_DEAD = 1<<6,
};

// should make Sharable always no-copy!
template<class T>
class PreventCopy : public T {
	PreventCopy() {}
	PreventCopy(const PreventCopy<T> &o) = delete;
	void operator=(const PreventCopy<T> &o) = delete;
};



template <class T>
class SharableX : public T {
	int _pointer_ref_counter = 0;
public:
	SharableX() {}
	SharableX(const SharableX<T> &o) = delete;
	void operator=(const SharableX<T> &o) = delete;
	auto _pointer_ref() {
		_pointer_ref_counter ++;
		pdb(format("ref %s -> %d", p2s(this), _pointer_ref_counter));
		return this;
	}
	void _pointer_unref() {
		_pointer_ref_counter --;
		pdb(format("unref %s -> %d", p2s(this), _pointer_ref_counter));
		if (_pointer_ref_counter < 0) {
			msg_error("---- OOOOOOO");
			crash();
			exit(1);
		}
	}
	bool _has_pointer_refs() {
		return _pointer_ref_counter > 0;
	}
};


class MidiNote : public SharableX<Empty> {
public:
	MidiNote();
	MidiNote(const Range &range, float pitch, float volume);
	MidiNote *copy() const;
	Range range;
	float pitch;
	float volume;
	int flags;
	int stringno;

	// temporary meta data
	mutable int clef_position;
	mutable NoteModifier modifier;
	mutable int y;

	bool is(int mask) const;
	void set(int mask);

	void reset_clef();
	void update_clef_pos(const Clef &clef, const Instrument &instrument, const Scale &s) const;
};


#endif /* SRC_DATA_MIDI_MIDINOTE_H_ */
