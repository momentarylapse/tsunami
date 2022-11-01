#include "rhythm.h"
#include "../MidiPainter.h"
#include "../../../data/rhythm/Bar.h"
#include "../../../data/midi/MidiData.h"
#include "../../audioview/ViewPort.h"


void get_col(color &col, color &col_shadow, const MidiNote *n, MidiNoteState state, bool playable, ColorScheme &colors);
MidiNoteState note_state(MidiNote *n, bool as_reference, SongSelection *sel, HoverData *hover);


QuantizedNote::QuantizedNote(){ n = nullptr; }
QuantizedNote::QuantizedNote(MidiNote *note, const Range &r, float spu) {
    n = note;
    offset = int((float)r.offset / spu + 0.5f);
    end = int((float)r.end() / spu + 0.5f);
    length = end - offset;
    base_length = length;
    x = y_min = y_max = 0;
    col = White;
    triplet = (length == 2) or (length == 4) or (length == 8);
    punctured = (length == 9) or (length == 18) or (length == 36);
    if (triplet)
        base_length = (base_length / 2) * 3;
    if (punctured)
        base_length = (base_length / 3) * 2;
    up = true;
}

int QuantizedNoteGroup::homogeneous() const {
    int l = notes[0].length;
    for (auto &d: notes)
        if (d.length != l)
            return -1;
    return l;
}


bool find_group(Array<QuantizedNote> &ndata, QuantizedNote &d, int i, QuantizedNoteGroup &group, int beat_end) {
	group.notes = {d};
	for (int j=i+1; j<ndata.num; j++) {
		// non-continguous?
		if (ndata[j].offset != ndata[j-1].end)
			break;
		if (ndata[j].triplet != d.triplet)
			break;
		// size mismatch?
		if ((ndata[j].length != TRIPLET_EIGHTH) and (ndata[j].length != TRIPLET_SIXTEENTH) and (ndata[j].length != 3*SIXTEENTH) and (ndata[j].length != EIGHTH) and (ndata[j].length != SIXTEENTH))
			break;
		// beat finished?
		if (ndata[j].end > beat_end)
			break;
		group.notes.add(ndata[j]);
	}
	return group.notes.back().end <= beat_end;
}

// ugly hard-coded quintuplet detection (TODO should use un-quantized data!)
bool find_group_x(Array<QuantizedNote> &ndata, QuantizedNote &d, int i, QuantizedNoteGroup &group, int beat_end) {
	if (ndata.num < i+5)
		return false;
	if (ndata[i].length != TRIPLET_SIXTEENTH)
		return false;
	if (ndata[i+1].length != SIXTEENTH)
		return false;
	if (ndata[i+2].length != TRIPLET_SIXTEENTH)
		return false;
	if (ndata[i+3].length != SIXTEENTH)
		return false;
	if (ndata[i+4].length != TRIPLET_SIXTEENTH)
		return false;
	if (ndata[i+4].end != beat_end)
		return false;

	for (int j=i+1; j<i+5; j++) {
		// non-continguous?
		if (ndata[j].offset != ndata[j-1].end)
			return false;
	}
	for (int j=i+1; j<i+5; j++)
		group.notes.add(ndata[j]);
	group.special_divisor = 5;
	return true;
}

QuantizedBar::QuantizedBar(Bar *b) {
    bar = b;
    int off = 0;
    for (int bb: b->beats) {
        int d = bb * QUARTER_PARTITION / b->divisor;
        beat_length.add(d);
        beat_offset.add(off);
        off += d;
    }
}
int QuantizedBar::get_beat(int offset) {
    for (int i=0; i<beat_offset.num; i++) {
        if (offset < beat_offset[i] + beat_length[i]) {
            return i;
        }
    }
    return -1;
}
int QuantizedBar::get_rel(int offset, int beat) {
    if (beat < 0)
        return -1;
    return offset - beat_offset[beat];
}
bool QuantizedBar::start_of_beat(int offset, int beat) {
    return get_rel(offset, beat) == 0;
}
bool QuantizedBar::allows_long_groups() {
    return bar->is_uniform() and (bar->beats[0] == bar->divisor) and (bar->beats.num % 2 == 0);
}

Array<QuantizedNote> quantize_all_notes_in_bar(MidiNoteBuffer &bnotes, Bar *b, MidiPainter *mp, float spu, std::function<float(MidiNote*)> y_func) {
	Array<QuantizedNote> ndata;
	float y_mid = (mp->area.y1 + mp->area.y2) / 2;

	for (MidiNote *n: weak(bnotes)) {
		Range r = n->range - b->offset;
		if (r.offset < 0)
			continue;
		auto d = QuantizedNote(n, r, spu);
		if (d.length == 0 or d.offset < 0)
			continue;

		//d.x = mp->cam->sample2screen(n->range.offset + mp->shift);
		d.x = mp->cam->sample2screen(n->range.center() + mp->shift);
		d.y_min = d.y_max = y_func(n);
		d.up = (d.y_min >= y_mid);


		color col_shadow;
		get_col(d.col, col_shadow, n, note_state(n, mp->as_reference, mp->sel, mp->hover), mp->is_playable, mp->local_theme);

		// prevent double notes
		if (ndata.num > 0)
			if (ndata.back().offset == d.offset) {
				// use higher note...
				if (d.n->pitch > ndata.back().n->pitch)
					std::swap(ndata.back(), d);
				ndata.back().y_min = min(ndata.back().y_min, d.y_min);
				ndata.back().y_max = max(ndata.back().y_max, d.y_max);
				continue;
			}

		ndata.add(d);
	}
	return ndata;
}

Array<QuantizedNoteGroup> digest_note_groups(Array<QuantizedNote> &ndata, QuantizedBar &qbar) {
	Array<QuantizedNoteGroup> groups;

	bool bar_allows_long_groups = qbar.allows_long_groups();

	//int offset = 0;
	for (int i=0; i<ndata.num; i++) {
		auto &d = ndata[i];

		// group start?
		int beat_index = qbar.get_beat(d.offset);
		int beat_end = qbar.beat_offset[beat_index] + qbar.beat_length[beat_index];
		if ((d.length == 3*SIXTEENTH or d.length == EIGHTH or d.length == SIXTEENTH or d.length == TRIPLET_EIGHTH or d.length == TRIPLET_SIXTEENTH)) {

			QuantizedNoteGroup group;
			group.starts_on_beat = qbar.start_of_beat(d.offset, beat_index);
			if (find_group(ndata, d, i, group, beat_end)) {
				if (group.starts_on_beat and group.notes.num == 1)
					find_group_x(ndata, d, i, group, beat_end);
				groups.add(group);
				i += group.notes.num - 1;
				continue;
			} else  {
				groups.add(group);
				i += group.notes.num - 1;
				continue;
			}
		}

		QuantizedNoteGroup group;
		group.notes.add(d);
		groups.add(group);
	}



	if (bar_allows_long_groups) {
		for (int i=0; i<groups.num-1; i++) {
			if (groups[i].starts_on_beat and groups[i+1].starts_on_beat) {
				if (groups[i].homogeneous() != EIGHTH)
					continue;
				if (groups[i+1].homogeneous() != EIGHTH)
					continue;
				if (groups[i].notes.back().end != groups[i+1].notes[0].offset)
					continue;
				if ((groups[i].notes[0].offset % (QUARTER_PARTITION*2)) != 0)
					continue;

				groups[i].notes.append(groups[i+1].notes);
				groups.erase(i+1);
			}
		}
		//allow_long_group = ((d.offset % (QUARTER_PARTITION*2)) == 0) and (d.length == EIGHTH);
	}

	return groups;
}
