/*
 * rhythm.h
 *
 *  Created on: 01.11.2022
 *      Author: michi
 */

#pragma once

#include "../../../data/midi/MidiNote.h"
#include "../../../lib/image/color.h"
#include <functional>

// rhythm quantization
static const int QUARTER_PARTITION = 12;
static const int QUARTER = QUARTER_PARTITION;
static const int EIGHTH = QUARTER_PARTITION / 2;
static const int SIXTEENTH = QUARTER_PARTITION / 4;
static const int TRIPLET_EIGHTH = QUARTER_PARTITION / 3;
static const int TRIPLET_SIXTEENTH = QUARTER_PARTITION / 6;

const float NOTE_NECK_LENGTH = 30.0f;
const float NOTE_BAR_DISTANCE = 10.0f;
const float NOTE_BAR_WIDTH = 5.0f;
const float NOTE_NECK_WIDTH = 2.0f;
const float NOTE_FLAG_DX = 10.0f;
const float NOTE_FLAG_DY = 15.0f;

class MidiNoteBuffer;
class MidiNote;
class MidiPainter;
class Bar;


struct QuantizedNote {
	QuantizedNote();
	QuantizedNote(MidiNote *note, const Range &r, float spu);

	float x, y_min, y_max;
	int offset, length, end;
	MidiNote *n;
	color col;
	bool triplet, punctured;
	int base_length; // drawing without triplet/punctured
	bool up;
};

class QuantizedNoteGroup {
public:
	Array<QuantizedNote> notes;
	bool starts_on_beat = false;
	int special_divisor = -1;

	int homogeneous() const;
};


bool find_group(Array<QuantizedNote> &ndata, QuantizedNote &d, int i, QuantizedNoteGroup &group, int beat_end);

bool find_group_x(Array<QuantizedNote> &ndata, QuantizedNote &d, int i, QuantizedNoteGroup &group, int beat_end);

struct QuantizedBar {
	Array<int> beat_offset;
	Array<int> beat_length;
	Bar *bar;
	QuantizedBar(Bar *b);
	int get_beat(int offset);
	int get_rel(int offset, int beat);
	bool start_of_beat(int offset, int beat);
	bool allows_long_groups();
};

Array<QuantizedNote> quantize_all_notes_in_bar(MidiNoteBuffer &bnotes, Bar *b, MidiPainter *mp, float spu, std::function<float(MidiNote*)> y_func);

Array<QuantizedNoteGroup> digest_note_groups(Array<QuantizedNote> &ndata, QuantizedBar &qbar);
