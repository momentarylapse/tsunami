/*
 * MidiPainter.cpp
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#include "MidiPainter.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h" // argh
#include "../audioview/ViewPort.h"
#include "../helper/SymbolRenderer.h"
#include "../ColorScheme.h"
#include "../../data/Song.h"
#include "../../data/midi/MidiData.h"
#include "../../data/midi/MidiNote.h"
#include "../../data/midi/Clef.h"
#include "../../data/midi/Instrument.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/SampleRef.h"
#include "../../data/Sample.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../module/ModuleConfiguration.h"


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


MidiKeyChange::MidiKeyChange(double _pos, const Scale &_key) : pos(_pos), key(_key) {}

MidiKeyChange::MidiKeyChange() : MidiKeyChange(0, Scale::C_MAJOR) {}



struct QuantizedNote {
	QuantizedNote(){ n = nullptr; }
	QuantizedNote(MidiNote *note, const Range &r, float spu) {
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

	int homogeneous() const {
		int l = notes[0].length;
		for (auto &d: notes)
			if (d.length != l)
				return -1;
		return l;
	}
};


MidiPainter::MidiPainter(Song *_song, ViewPort *_cam, SongSelection *_sel, HoverData *_hover, ColorScheme &_colors) :
	midi_scale(Scale::C_MAJOR),
	local_theme(_colors)
{
	song = _song;
	cam = _cam;
	sel = _sel;
	if (!sel)
		sel = new SongSelection; // TODO... delete later...
	hover = _hover;
	if (!hover)
		hover = new HoverData;
	instrument = nullptr;
	clef = nullptr;
	is_playable = true;
	as_reference = false;
	allow_shadows = true;
	force_shadows = false;
	pitch_min = PITCH_MIN_DEFAULT;
	pitch_max = PITCH_MAX_DEFAULT;
	shift = 0;
	clef_dy = 0;
	clef_y0 = 0;
	string_dy = 0;
	string_y0 = 0;
	mode = MidiMode::LINEAR;
	rr = 5;
	set_quality(1.0f, true);
}


void MidiPainter::__init__(Song *_song, ViewPort *_cam, SongSelection *_sel, HoverData *_hover, ColorScheme &_colors) {
	new(this) MidiPainter(_song, _cam, _sel, _hover, _colors);
}

color hash_color(int h) {
	return PITCH_COLORS[((h) & 0x0fffffff) % 12];
}

color MidiPainter::pitch_color(int pitch) {
	return PITCH_COLORS[pitch % 12];
	//return color::hsb((float)(pitch % 12) / 12.0f, 0.6f, 1, 1);
}


void get_col(color &col, color &col_shadow, const MidiNote *n, MidiPainter::MidiNoteState state, bool playable, ColorScheme &colors) {
	if (playable)
		col = colors.pitch_text[(int)n->pitch % 12];
	else
		col = colors.text_soft3;

	if (state & MidiPainter::STATE_REFERENCE)
		col = color::interpolate(col, colors.background_track, 0.65f);
	if (state & MidiPainter::STATE_SELECTED)
		col = color::interpolate(col, colors.selection, 0.5f);
	if (state & MidiPainter::STATE_HOVER)
		col = color::interpolate(col, colors.hover, 0.5f);

	col_shadow = color::interpolate(col, colors.background_track, 0.3f);
	//col_shadow = col;
	//col_shadow.a = 0.5f;
}


inline MidiPainter::MidiNoteState note_state(MidiNote *n, bool as_reference, SongSelection *sel, HoverData *hover) {
	MidiPainter::MidiNoteState s = MidiPainter::STATE_DEFAULT;
	if (sel->has(n))
		s = MidiPainter::STATE_SELECTED;
	if (as_reference)
		return (MidiPainter::MidiNoteState)(MidiPainter::STATE_REFERENCE | s);
	if ((hover->type == HoverData::Type::MIDI_NOTE) and (n == hover->note))
		return (MidiPainter::MidiNoteState)(MidiPainter::STATE_HOVER | s);
	return s;
}


float MidiPainter::clef_pos_to_screen(int pos) {
	return area.y2 - area.height() / 2 - (pos - 4) * clef_dy / 2.0f;
}

int MidiPainter::screen_to_clef_pos(float y) {
	return (int)floor((area.y2 - y - area.height() / 2) * 2.0f / clef_dy + 0.5f) + 4;
}

float MidiPainter::string_to_screen(int string_no) {
	return string_y0 - string_no * string_dy;
}

int MidiPainter::screen_to_string(float y) {
	return (int)floor((string_y0 - y) / string_dy + 0.5f);
}

// this is the "center" of the note!
// it reaches below and above this y
float MidiPainter::pitch2y_linear(float pitch) {
	return area.y2 - area.height() * (pitch - (float)pitch_min) / (pitch_max - pitch_min);
}

float MidiPainter::pitch2y_classical(int pitch) {
	NoteModifier mod;
	int p = clef->pitch_to_position(pitch, midi_scale, mod);
	return clef_pos_to_screen(p);
}

int MidiPainter::y2pitch_linear(float y) {
	return pitch_min + ((area.y2 - y) * (pitch_max - pitch_min) / area.height()) + 0.5f;
}

int MidiPainter::y2pitch_classical(float y, NoteModifier modifier) {
	int pos = screen_to_clef_pos(y);
	return clef->position_to_pitch(pos, midi_scale, modifier);
}

int MidiPainter::y2clef_classical(float y, NoteModifier &mod) {
	mod = NoteModifier::UNKNOWN;//modifier;
	return screen_to_clef_pos(y);
}

int MidiPainter::y2clef_linear(float y, NoteModifier &mod) {
	mod = NoteModifier::UNKNOWN;//modifier;

	int pitch = y2pitch_linear(y);
	return clef->pitch_to_position(pitch, midi_scale, mod);
}

void MidiPainter::draw_single_ndata(Painter *c, QuantizedNote &d, bool neck_offset) {
	c->set_color(d.col);
	float e = d.up ? -1 : 1;
	float y = d.up ? d.y_min : d.y_max;

	float neck_dy0 = 0;
	if (neck_offset)
		neck_dy0 = e * rr * 1.5f;

	if (d.base_length == SIXTEENTH) {
		c->set_line_width(neck_width);
		c->draw_line({d.x, y + neck_dy0}, {d.x, y + e * neck_length_single});
		c->set_line_width(bar_width);
		c->draw_line({d.x, y + e * neck_length_single}, {d.x + flag_dx, y + e * (neck_length_single - flag_dy)});
		c->draw_line({d.x, y + e * (neck_length_single - bar_distance)}, {d.x + flag_dx, y + e * (neck_length_single - bar_distance - flag_dy)});
	} else if (d.base_length == EIGHTH) {
		c->set_line_width(neck_width);
		c->draw_line({d.x, y + neck_dy0}, {d.x, y + e * neck_length_single});
		c->set_line_width(bar_width);
		c->draw_line({d.x, y + e * neck_length_single}, {d.x + flag_dx, y + e * (neck_length_single - flag_dy)});
	} else if (d.base_length == QUARTER) {
		c->set_line_width(neck_width);
		c->draw_line({d.x, y + neck_dy0}, {d.x, y + e * neck_length_single});
	}

	if (d.punctured)
		c->draw_circle({d.x + rr, y + rr}, 2);
	if (d.triplet)
		c->draw_str({d.x, y + e*neck_length_single + e * 9 - 4}, "3");
}

void MidiPainter::draw_group_ndata(Painter *c, const QuantizedNoteGroup &d, bool neck_offset) {
	bool up = d.notes[0].up;
	float e = up ? -1 : 1;
	float x0 = d.notes[0].x;
	float y0 = up ? d.notes[0].y_min : d.notes[0].y_max;
	float x1 = d.notes.back().x;
	float y1 = up ? d.notes.back().y_min : d.notes.back().y_max;
	float dx = x1 - x0;
	float dy = y1 - y0;
	float m = dy / dx;

	// optimize neck length
	float dy0min = 0;
	for (auto &dd: d.notes)
		dy0min = max(dy0min, e * (dd.y_min - (y0 + m * (dd.x - x0))));
	y0 += e * max(neck_length_group, dy0min + neck_length_group * 0.8f);

	// draw necks
	float neck_dy0 = 0;
	if (neck_offset)
		neck_dy0 = e * rr * 1.5f;

	c->set_line_width(neck_width);
	for (auto &dd: d.notes)
		if (dd.n) {
			c->set_color(dd.col);
			float yy = up ? dd.y_min : dd.y_max;
			c->draw_line({dd.x, yy + neck_dy0}, {dd.x, y0 + m * (dd.x - x0)});
		}

	// bar
	c->set_line_width(bar_width);
	float t0 = 0;
	for (int i=0; i<d.notes.num; i++) {
		float xx = d.notes[i].x;
		if (i+1 < d.notes.num) {
			if (d.notes[i+1].length == d.notes[i].length)
				xx = (d.notes[i+1].x + d.notes[i].x)/2;
				//xx = (d[i+1].x * d[i].length + d[i].x * d[i+1].length)/(d[i].length + d[i+1].length);
			if (d.notes[i+1].length < d.notes[i].length)
				xx = d.notes[i+1].x;
		}
		if (xx == x0)
			xx = (x0*2 + d.notes[1].x) / 3;
		if (xx == x1 and (i+1 < d.notes.num))
			xx = (x1*4 + d.notes[1-1].x) / 5;
		float t1 = (xx - x0) / dx;
		if (d.notes[i].n){
			c->set_color(d.notes[i].col);
			c->draw_line({x0 + dx*t0, y0 + dy*t0}, {x0 + dx*t1, y0 + dy*t1});
			if (d.notes[i].length <= SIXTEENTH)
				c->draw_line({x0 + dx*t0, y0 + dy*t0 - e*bar_distance}, {x0 + dx*t1, y0 + dy*t1 - e*bar_distance});
			if (d.notes[i].punctured)
				c->draw_circle({d.notes[i].x + rr, d.notes[i].y_min + rr}, 2);
		}
		t0 = t1;
	}

	int div = d.special_divisor;
	if (div < 0 and d.notes[0].triplet)
		div = d.notes[0].length == TRIPLET_SIXTEENTH ? 6 : 3;
	if (div > 0)
		c->draw_str({x0 + dx/2, y0 + dy/2 + c->font_size * (e*1.3f - 0.5f)}, i2s(div));
	//else if (d.notes[0].triplet)
		//c->draw_str((x0 + x1)/2, (y0 + y1)/2 - 4 + e*9, "3");
	//	c->draw_str({x0 + dx/2, y0 + dy/2 + c->font_size * (e*1.3f - 0.5f)}, "3");
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

struct QuantizedBar {
	Array<int> beat_offset;
	Array<int> beat_length;
	Bar *bar;
	QuantizedBar(Bar *b) {
		bar = b;
		int off = 0;
		for (int bb: b->beats) {
			int d = bb * QUARTER_PARTITION / b->divisor;
			beat_length.add(d);
			beat_offset.add(off);
			off += d;
		}
	}
	int get_beat(int offset) {
		for (int i=0; i<beat_offset.num; i++) {
			if (offset < beat_offset[i] + beat_length[i]) {
				return i;
			}
		}
		return -1;
	}
	int get_rel(int offset, int beat) {
		if (beat < 0)
			return -1;
		return offset - beat_offset[beat];
	}
	bool start_of_beat(int offset, int beat) {
		return get_rel(offset, beat) == 0;
	}
	bool allows_long_groups() {
		return bar->is_uniform() and (bar->beats[0] == bar->divisor) and (bar->beats.num % 2 == 0);
	}
};

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


void MidiPainter::draw_rhythm(Painter *c, const MidiNoteBuffer &midi, const Range &range, std::function<float(MidiNote*)> y_func) {
	if (cam->pixels_per_sample < quality.rhythm_zoom_min)
		return;

	c->set_antialiasing(quality.antialiasing);

	/*if (vlayer->is_playable())
		c->set_color(colors.text_soft1);
	else
		c->set_color(colors.text_soft3);*/


	bool neck_offset = (mode == MidiMode::TAB);

	auto bars = song->bars.get_bars(range);
	for (auto *b: bars) {
		if (b->is_pause())
			continue;

		// samples per 16th / 3
		float sub_beat_length = (float)b->range().length / (float)b->total_sub_beats;
		float quarter_length = sub_beat_length * b->divisor;
		float spu = quarter_length / (float)QUARTER_PARTITION;
//		float spu = (float)b->range().length / (float)b->num_beats / (float)BEAT_PARTITION;

		auto bnotes = midi.get_notes(b->range());
		//c->set_color(colors.text_soft3);

		auto ndata = quantize_all_notes_in_bar(bnotes, b, this, spu, y_func);

		QuantizedBar qbar(b);

		auto groups = digest_note_groups(ndata, qbar);

		for (auto &g: groups) {
			if (g.notes.num == 1)
				draw_single_ndata(c, g.notes[0], neck_offset);
			else
				draw_group_ndata(c, g, neck_offset);
		}

	}
	c->set_line_width(1);
	c->set_antialiasing(false);
}



void MidiPainter::draw_pitch_grid(Painter *c, Synthesizer *synth) {
	// pitch grid
	c->set_color(color(0.25f, 0, 0, 0));
	for (int i=pitch_min; i<pitch_max; i++) {
		float y0 = pitch2y_linear(i + 0.5f);
		float y1 = pitch2y_linear(i - 0.5f);
		if (!midi_scale.contains(i)) {
			c->set_color(color(0.2f, 0, 0, 0));
			c->draw_rect(rect(area.x1, area.x2, y0, y1));
		}
	}


	// pitch names
	color cc = local_theme.text;
	cc.a = 0.4f;
	Array<SampleRef*> *p = nullptr;
	if (synth and (synth->module_class == "Sample")) {
		auto *c = synth->get_config();
		p = (Array<SampleRef*> *)&c[1];
	}
	bool is_drum = (instrument->type == Instrument::Type::DRUMS);
	float dy = ((pitch2y_linear(0) - pitch2y_linear(1)) - c->font_size) / 2;
	for (int i=pitch_min; i<pitch_max; i++) {
		c->set_color(cc);
		if (((hover->type == HoverData::Type::MIDI_PITCH) and (i == hover->index))) // or ((hover->type == HoverData::Type::MIDI_NOTE) and (i == hover->pitch)))
			c->set_color(local_theme.text);

		string name = pitch_name(i);
		if (is_drum) {
			name = drum_pitch_name(i);
		} else if (p) {
			if (i < p->num)
				if ((*p)[i])
					name = (*p)[i]->origin->name;
		}
		c->draw_str({20, pitch2y_linear(i+0.5f)+dy}, name);
	}
}

void MidiPainter::draw_note_flags(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y) {

	if (n->flags > 0) {
		float x = (x1 + x2) / 2;
		if (n->is(NOTE_FLAG_DEAD))
			SymbolRenderer::draw(c, {x, y - rr*4}, rr*1.5f, "x", true, 0);
		else if (n->is(NOTE_FLAG_TRILL))
			SymbolRenderer::draw(c, {x, y - rr*4}, rr*1.5f, "tr", true, 0);
			//c->draw_str(x, y - rr*3, "tr~");
		if (n->is(NOTE_FLAG_STACCATO))
			c->draw_circle({x + rr, y + rr*2}, 2);
		if (n->is(NOTE_FLAG_TENUTO)) {
			c->set_line_width(3);
			c->draw_line({x, y + rr*2}, {x1+20, y + rr*2});
		}
	}

}

void MidiPainter::draw_complex_note(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y) {
	if (state & MidiPainter::STATE_SELECTED) {
		color col1 = local_theme.selection;
		draw_simple_note(c, x1, x2, y, 2, col1, col1, false);
	}

	color col, col_shadow;
	get_col(col, col_shadow, n, state, is_playable, local_theme);

	draw_simple_note(c, x1, x2, y, 0, col, col_shadow, false);

	draw_note_flags(c, n, state, x1, x2, y);
}


void MidiPainter::draw_note_linear(Painter *c, const MidiNote &n, MidiNoteState state) {
	float x1, x2;
	cam->range2screen(n.range, x1, x2);
	float y = pitch2y_linear(n.pitch);
	n.y = y;

	draw_complex_note(c, &n, state, x1, x2, y);
}

void MidiPainter::draw_linear(Painter *c, const MidiNoteBuffer &notes) {
	draw_rhythm(c, notes, cur_range, [=](MidiNote *n){ return pitch2y_linear(n->pitch); });
	//c->setLineWidth(3.0f);

	// draw notes
	c->set_antialiasing(quality.antialiasing);
	for (auto *n: weak(notes)) {
		if ((n->pitch < pitch_min) or (n->pitch >= pitch_max))
			continue;
		draw_note_linear(c, *n, note_state(n, as_reference, sel, hover));
	}
	c->set_antialiasing(false);
}


// "shadow" to indicate length
void draw_shadow(Painter *c, float x1, float x2, float y, float rx, float rr, const color &col) {
	//x1 += r;
	c->set_color(col);
	c->draw_rect(rect(x1, x2 + rx, y - rr*0.7f - rx, y + rr*0.7f + rx));
}

void draw_shadow2(Painter *c, float x1, float x2, float y, float dx, float clef_line_width, const color &col) {
	c->set_color(col);
	c->set_line_width(3 * clef_line_width);
	float x = (x1 + x2) / 2;
	c->draw_line({x1, y}, {x - dx, y});
	c->draw_line({x + dx, y}, {x2, y});
}

void MidiPainter::draw_simple_note(Painter *c, float x1, float x2, float y, float rx, const color &col, const color &col_shadow, bool force_circle) {
	//x1 += r;
	// "shadow" to indicate length
	if (allow_shadows and (x2 - x1 > quality.shadow_threshold))
		//draw_shadow(c, x1, x2, y, rx, rr, col_shadow);
		draw_shadow2(c, x1, x2, y, rr * 2, clef_line_width, col_shadow);

	// the note circle
	c->set_color(col);
	if ((x2 - x1 > quality.note_circle_threshold) or force_circle)
		c->draw_circle({(x1+x2)/2, y}, rr+rx);
	else
		c->draw_rect(rect(x1 - rr*0.8f - rx, x1 + rr*0.8f + rx, y - rr*0.8f - rx, y + rr*0.8f + rx));
}

void MidiPainter::draw_clef_tab(Painter *c) {
	if (is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);
	c->set_line_width(clef_line_width);
	c->set_antialiasing(true);

	// clef lines
	float h = string_dy * instrument->string_pitch.num;
	for (int i=0; i<instrument->string_pitch.num; i++) {
		float y = string_to_screen(i);
		c->draw_line({area.x1, y}, {area.x2, y});
	}
	c->set_antialiasing(false);


	if (is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);

	c->set_font_size(h / 6);
	c->draw_str({10, area.y1 + area.height() / 2 - h * 0.37f}, "T\nA\nB");
	c->set_font_size(local_theme.FONT_SIZE);
}

void MidiPainter::draw_note_tab(Painter *c, const MidiNote *n, MidiNoteState state) {
	float x1, x2;
	cam->range2screen(n->range + shift, x1, x2);


	int p = n->stringno;
	float y = string_to_screen(p);
	n->y = y;

	color col, col_shadow;
	get_col(col, col_shadow, n, state, is_playable, local_theme);
	//draw_complex_note(c, n, state, x1, x2, y);

	float x = (x1 + x2) / 2;
	float font_size = rr * 1.6f;

	if (n->is(NOTE_FLAG_DEAD)) {
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, "x", true, 0);
		return;
	}

	if (x2 - x1 > quality.tab_text_threshold /*and rr > 5*/) {
		string tt = i2s(n->pitch - instrument->string_pitch[n->stringno]);
		float dx = rr * 0.8f * tt.num;

		// "shadow" to indicate length
		if (allow_shadows and (x2 - x1 > quality.shadow_threshold*1.5f)) {
			//draw_shadow(c, x1, x2, y, 2, rr, col, col_shadow);
			dx += rr * 1.0f;
			draw_shadow2(c, x1, x2, y, dx, clef_line_width, col_shadow);
		}

		// hide the string line to make the number more readable
		color cc = local_theme.background_track;
		cc.a = 0.5f;

		c->set_color(cc);
		c->draw_rect(rect(x - dx, x + dx, y - 2, y + 2));

		// fret number as symbol
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, tt, true, 0);

		draw_note_flags(c, n, state, x1, x2, y);
	} else {
		string tt = i2s(n->pitch - instrument->string_pitch[n->stringno]);
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, tt, true, 0);
		draw_note_flags(c, n, state, x1, x2, y);
	}
}

void MidiPainter::draw_tab(Painter *c, const MidiNoteBuffer &notes) {
	draw_rhythm(c, notes, cur_range, [=](MidiNote *n){ return string_to_screen(n->stringno); });

	c->set_antialiasing(quality.antialiasing);
	for (auto *n: weak(notes))
		draw_note_tab(c,  n,  note_state(n, as_reference, sel, hover));
	c->set_antialiasing(false);

	c->set_font_size(local_theme.FONT_SIZE);
}

void MidiPainter::draw_note_classical(Painter *c, const MidiNote *n, MidiNoteState state) {
	float x1, x2;
	cam->range2screen(n->range + shift, x1, x2);
	float x = (x1 + x2) / 2;

	// checked before...
//	if (n.clef_position < 0)
//		n.update_meta(track->instrument, midi_scale, 0);

	int p = n->clef_position;
	float y = clef_pos_to_screen(p);
	n->y = y;

	// auxiliary lines
	for (int i=10; i<=p; i+=2) {
		c->set_line_width(clef_line_width);
		c->set_color(local_theme.text_soft2);
		float y = clef_pos_to_screen(i);
		c->draw_line({x - clef_dy, y}, {x + clef_dy, y});
	}
	for (int i=-2; i>=p; i-=2) {
		c->set_line_width(clef_line_width);
		c->set_color(local_theme.text_soft2);
		float y = clef_pos_to_screen(i);
		c->draw_line({x - clef_dy, y}, {x + clef_dy, y});
	}

	draw_complex_note(c, n, state, x1, x2, y);

	if ((n->modifier != NoteModifier::NONE) and (rr >= 3)) {
		c->set_color(local_theme.text);
		//c->setColor(ColorInterpolate(col, colors.text, 0.5f));
		SymbolRenderer::draw(c, {x - modifier_font_size*1.0f, y - modifier_font_size*0.5f}, modifier_font_size, modifier_symbol(n->modifier));
	}
}

void MidiPainter::draw_key_symbol(Painter *c, const MidiKeyChange &kc) {
	float x = cam->sample2screen(kc.pos);

	c->set_font_size(clef_dy*4);
	c->draw_str({x + 10, clef_pos_to_screen(8)}, clef->symbol);


	//SymbolRenderer::draw(c, x - size*1.0f, y - size*0.5f , size, modifier_symbol(n->modifier));
	//c->set_font_size(clef_dy);

	for (int i=0; i<7; i++) {
		if (kc.key.modifiers[i] != NoteModifier::NONE)
			SymbolRenderer::draw(c, {x + 18 + modifier_font_size*3.0f + modifier_font_size*0.6f*(i % 3), clef_pos_to_screen((i - clef->offset + 7*20) % 7) - modifier_font_size/2}, modifier_font_size, modifier_symbol(kc.key.modifiers[i]));
	}
	c->set_font_size(local_theme.FONT_SIZE);
}

void MidiPainter::draw_clef_classical(Painter *c) {
	// clef lines

	if (is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);
	c->set_line_width(clef_line_width);
	c->set_antialiasing(true);

	for (int i=0; i<10; i+=2) {
		float y = clef_pos_to_screen(i);
		c->draw_line({area.x1, y}, {area.x2, y});
	}
	c->set_antialiasing(false);
	
	if (is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);

	// clef symbol

	Scale key_prev = Scale::C_MAJOR;
	for (auto &kc: key_changes)
		if (kc.pos < cam->range().offset)
			key_prev = kc.key;
	draw_key_symbol(c, MidiKeyChange(cam->pos, key_prev));

	for (auto &kc: key_changes)
		draw_key_symbol(c, kc);

	c->set_font_size(local_theme.FONT_SIZE);
}



void MidiPainter::draw_classical(Painter *c, const MidiNoteBuffer &notes) {
	draw_rhythm(c, notes, cur_range, [=](MidiNote *n){ return clef_pos_to_screen(n->clef_position); });

	c->set_antialiasing(quality.antialiasing);
	for (auto *n: weak(notes))
		draw_note_classical(c, n, note_state(n, as_reference, sel, hover));
	c->set_antialiasing(false);

	c->set_font_size(local_theme.FONT_SIZE);
}

void MidiPainter::draw_low_detail_dummy(Painter *c, const MidiNoteBuffer &notes) {
	auto bars = song->bars.get_bars(cur_range);

	// along bars
	for (auto *b: bars)
		draw_low_detail_dummy_part(c, b->range(), notes.get_notes(b->range()));

	int dpos = song->sample_rate * 5; // 5s intervals

	// before bars
	for (int pos=song->bars.range().start()-dpos; pos>=cur_range.start()-dpos; pos-=dpos)
		draw_low_detail_dummy_part(c, Range(pos, dpos), notes.get_notes(Range(pos, dpos)));
	// after bars
	for (int pos=song->bars.range().end(); pos<cur_range.end(); pos+=dpos)
		draw_low_detail_dummy_part(c, Range(pos, dpos), notes.get_notes(Range(pos, dpos)));
}

void MidiPainter::draw_low_detail_dummy_part(Painter *c, const Range &r, const MidiNoteBuffer &notes) {
	if (notes.num == 0)
		return;
	float x0, x1;
	cam->range2screen(r, x0, x1);
	int count[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	for (MidiNote *n: weak(notes))
		count[pitch_to_rel(n->pitch)] ++;
	for (int i=0; i<12; i++) {
		if (count[i] == 0)
			continue;
		color col = local_theme.pitch[i];
		if (!is_playable)
			col = color::interpolate(col, local_theme.text_soft3, 0.8f);
		col.a = (float)count[i] / (float)notes.num;
		c->set_color(col);
		float y0 = area.y2 - i*area.height()/12;
		float y1 = area.y2 - (i+1)*area.height()/12;
		c->draw_rect(rect(x0, x1, y0, y1));
	}
}

Range extend_range_to_bars(const Range &r, const BarCollection &bars) {
	Range rr = r;
	for (auto b: weak(bars)) {
		if (b->range().is_more_inside(rr.start()))
			rr.set_start(b->range().start());
		if (b->range().is_more_inside(rr.end()))
			rr.set_end(b->range().end());
	}
	return rr;
}

void MidiPainter::_draw_notes(Painter *p, const MidiNoteBuffer &notes) {

	if (mode == MidiMode::LINEAR)
		draw_linear(p, notes);
	else if (mode == MidiMode::TAB)
		draw_tab(p, notes);
	else // if (mode == MidiMode::CLASSICAL)
		draw_classical(p, notes);
}

void MidiPainter::draw(Painter *c, const MidiNoteBuffer &midi) {
	auto xxx = c->clip();
	c->set_clip(area and c->area());
	cur_range = extend_range_to_bars(cam->range() - shift, song->bars);
	midi.update_clef_pos(*instrument, midi_scale);
	auto notes = midi.get_notes(cur_range);

	if (cam->pixels_per_sample < quality.notes_zoom_min) {
		draw_low_detail_dummy(c, midi);
	} else {
		_draw_notes(c, notes);
	}
	c->set_line_width(1);
	c->set_clip(xxx);
}

void MidiPainter::draw_background(Painter *c) {
	if (mode == MidiMode::CLASSICAL) {
		draw_clef_classical(c);
	} else if (mode == MidiMode::TAB) {
		draw_clef_tab(c);
	}
}

void MidiPainter::set_context(const rect& _area, const Instrument& i, bool _is_playable, MidiMode _mode) {
	area = _area;
	instrument = &i;
	clef = &i.get_clef();

	// TAB
	string_dy = min((area.height() * 0.7f) / max(6, instrument->string_pitch.num), 40.0f);
	float h = string_dy * instrument->string_pitch.num;
	string_y0 = area.y2 - (area.height() - h) / 2 - string_dy/2;

	// classical clef
	clef_dy = min(area.height() / 13, 30.0f);
	clef_y0 = area.y2 - area.height() / 2 + 2 * clef_dy;

	clef_line_width = area.height() / 150;

	mode = _mode;
	is_playable = _is_playable;
	as_reference = false;
	shift = 0;
	allow_shadows = true;//(mode == MidiMode::LINEAR);
	force_shadows = false;

	pitch_min = PITCH_MIN_DEFAULT;
	pitch_max = PITCH_MAX_DEFAULT;

	set_line_weight(1.0f);
}

void MidiPainter::set_line_weight(float s) {
	scale = s;

	if (mode == MidiMode::CLASSICAL)
		rr = min(clef_dy * 0.42f, 10.0f);
	if (mode == MidiMode::LINEAR)
		rr = max((pitch2y_linear(0) - pitch2y_linear(1)) / 1.3f, 2.0f);
	if (mode == MidiMode::TAB)
		rr = min(string_dy/2, 13.0f);

	modifier_font_size = rr * 2.8f;

	neck_length_single = max(NOTE_NECK_LENGTH, rr*7);
	neck_length_group = max(NOTE_NECK_LENGTH, rr*6);
	neck_width = max(NOTE_NECK_WIDTH * scale, rr/3);
	bar_distance = NOTE_BAR_DISTANCE * scale;
	bar_width = NOTE_BAR_WIDTH * scale;
	flag_dx = NOTE_FLAG_DX * scale;
	flag_dy = NOTE_FLAG_DY * scale;
}

void MidiPainter::set_key_changes(const Array<MidiKeyChange> &changes) {
	key_changes = changes;
}

void MidiPainter::set_linear_range(float _pitch_min, float _pitch_max) {
	pitch_min = _pitch_min - 0.5f;
	pitch_max = _pitch_max - 0.5f;
}

void MidiPainter::set_shift(int _shift) {
	shift = _shift;
}

void MidiPainter::set_quality(float q, bool antialiasing) {
	quality.factor = q;
	quality.rhythm_zoom_min = 40 / q / song->sample_rate;
	quality.notes_zoom_min = 2 / q / song->sample_rate;
	quality.shadow_threshold = rr*4.0f / q;
	quality.note_circle_threshold = 6 / q;
	quality.tab_text_threshold = rr/4 / q;
	quality.antialiasing = antialiasing;
	quality.note_count_threshold = area.width() * 0.4f * q;
}
void MidiPainter::set_force_shadows(bool force) {
	force_shadows = force;
	allow_shadows = force;// or (mode == MidiMode::LINEAR);
}
