/*
 * MidiPainter.cpp
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#include "MidiPainter.h"
#include "../AudioView.h"
#include "../ViewPort.h"
#include "../ColorScheme.h"
#include "../Node/AudioViewLayer.h" // argh
#include "../../Data/Song.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/Midi/MidiNote.h"
#include "../../Data/Midi/Clef.h"
#include "../../Data/Midi/Instrument.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/SampleRef.h"
#include "../../Data/Sample.h"
#include "../Helper/SymbolRenderer.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/ModuleConfiguration.h"


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

static const color PITCH_COLORS[12] = {
	color(1, 1.000000, 0.400000, 0.400000), // C
	color(1, 0.900000, 0.700000, 0.400000),
	color(1, 0.800000, 0.800000, 0.400000), // D
	color(1, 0.700000, 0.900000, 0.400000),
	color(1, 0.400000, 0.900000, 0.400000), // E
	color(1, 0.400000, 0.900000, 0.700000), // F
	color(1, 0.400000, 0.900000, 1.000000),
	color(1, 0.400000, 0.700000, 1.000000), // G
	color(1, 0.400000, 0.400000, 1.000000),
	color(1, 0.700000, 0.400000, 1.000000), // A
	color(1, 1.000000, 0.400000, 1.000000),
	color(1, 1.000000, 0.400000, 0.700000)  // B
};

MidiKeyChange::MidiKeyChange(int _pos, const Scale &_key) : pos(_pos), key(_key) {}

MidiKeyChange::MidiKeyChange() : MidiKeyChange(0, Scale::C_MAJOR) {}

MidiPainter::MidiPainter(AudioView *view) : MidiPainter(view->song, &view->cam, &view->sel, &view->hover, view->colors)
{}

MidiPainter::MidiPainter(Song *_song, ViewPort *_cam, SongSelection *_sel, HoverData *_hover, ColorScheme &_colors) :
	midi_scale(Scale::C_MAJOR),
	colors(_colors)
{
	song = _song;
	cam = _cam;
	sel = _sel;
	hover = _hover;
	instrument = nullptr;
	clef = nullptr;
	is_playable = true;
	as_reference = false;
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


color MidiPainter::pitch_color(int pitch)
{
	return PITCH_COLORS[pitch % 12];
	//return SetColorHSB(1, (float)(pitch % 12) / 12.0f, 0.6f, 1);
}


void get_col(color &col, color &col_shadow, const MidiNote *n, MidiPainter::MidiNoteState state, bool playable, ColorScheme &colors)
{
	if (playable)
		col = ColorInterpolate(MidiPainter::pitch_color(n->pitch), colors.text, 0.2f);
	else
		col = colors.text_soft3;

	if (state & MidiPainter::STATE_HOVER){
		col = ColorInterpolate(col, colors.hover, 0.5f);
	}else if (state & MidiPainter::STATE_SELECTED){
		col = ColorInterpolate(col, colors.selection, 0.5f);
	}else if (state & MidiPainter::STATE_REFERENCE){
		col = ColorInterpolate(col, colors.background_track, 0.65f);
	}
	col_shadow = ColorInterpolate(col, colors.background_track, 0.3f);
}


inline MidiPainter::MidiNoteState note_state(MidiNote *n, bool as_reference, SongSelection *sel, HoverData *hover)
{
	MidiPainter::MidiNoteState s = MidiPainter::STATE_DEFAULT;
	if (sel->has(n))
		s = MidiPainter::STATE_SELECTED;
	if (as_reference)
		return (MidiPainter::MidiNoteState)(MidiPainter::STATE_REFERENCE | s);
	if ((hover->type == HoverData::Type::MIDI_NOTE) and (n == hover->note))
		return (MidiPainter::MidiNoteState)(MidiPainter::STATE_HOVER | s);
	return s;
}


float MidiPainter::clef_pos_to_screen(int pos)
{
	return area.y2 - area.height() / 2 - (pos - 4) * clef_dy / 2.0f;
}

int MidiPainter::screen_to_clef_pos(float y)
{
	return (int)floor((area.y2 - y - area.height() / 2) * 2.0f / clef_dy + 0.5f) + 4;
}

float MidiPainter::string_to_screen(int string_no)
{
	return string_y0 - string_no * string_dy;
}

int MidiPainter::screen_to_string(float y)
{
	return (int)floor((string_y0 - y) / string_dy + 0.5f);
}

float MidiPainter::pitch2y_linear(int pitch)
{
	return area.y2 - area.height() * ((float)pitch - pitch_min) / (pitch_max - pitch_min);
}

float MidiPainter::pitch2y_classical(int pitch)
{
	NoteModifier mod;
	int p = clef->pitch_to_position(pitch, midi_scale, mod);
	return clef_pos_to_screen(p);
}

int MidiPainter::y2pitch_linear(float y)
{
	return pitch_min + ((area.y2 - y) * (pitch_max - pitch_min) / area.height());
}

int MidiPainter::y2pitch_classical(float y, NoteModifier modifier)
{
	int pos = screen_to_clef_pos(y);
	return clef->position_to_pitch(pos, midi_scale, modifier);
}

int MidiPainter::y2clef_classical(float y, NoteModifier &mod)
{
	mod = NoteModifier::UNKNOWN;//modifier;
	return screen_to_clef_pos(y);
}

int MidiPainter::y2clef_linear(float y, NoteModifier &mod)
{
	mod = NoteModifier::UNKNOWN;//modifier;

	int pitch = y2pitch_linear(y);
	return clef->pitch_to_position(pitch, midi_scale, mod);
}

struct NoteData
{
	NoteData(){ n = nullptr; }
	NoteData(MidiNote *note, const Range &r, float spu)
	{
		n = note;
		offset = int((float)r.offset / spu + 0.5f);
		end = int((float)r.end() / spu + 0.5f);
		length = end - offset;
		base_length = length;
		x = y = 0;
		col = White;
		triplet = (length == 2) or (length == 4) or (length == 8);
		punctured = (length == 9) or (length == 18) or (length == 36);
		if (triplet)
			base_length = (base_length / 2) * 3;
		if (punctured)
			base_length = (base_length / 3) * 2;
		up = true;
	}
	float x, y;
	int offset, length, end;
	MidiNote *n;
	color col;
	bool triplet, punctured;
	int base_length; // drawing without triplet/punctured
	bool up;
};

void draw_single_ndata(Painter *c, NoteData &d, float rr)
{
	float neck_length = max(NOTE_NECK_LENGTH, rr*5);
	float neck_width = max(NOTE_NECK_WIDTH, rr/3);
	c->set_color(d.col);
	float e = d.up ? -1 : 1;
	if (d.base_length == SIXTEENTH){
		c->set_line_width(neck_width);
		c->draw_line(d.x, d.y, d.x, d.y + e * neck_length);
		c->set_line_width(NOTE_BAR_WIDTH);
		c->draw_line(d.x, d.y + e * neck_length, d.x + NOTE_FLAG_DX, d.y + e * (neck_length - NOTE_FLAG_DY));
		c->draw_line(d.x, d.y + e * (neck_length - NOTE_BAR_DISTANCE), d.x + NOTE_FLAG_DX, d.y + e * (neck_length - NOTE_BAR_DISTANCE - NOTE_FLAG_DY));
	}else if (d.base_length == EIGHTH){
		c->set_line_width(neck_width);
		c->draw_line(d.x, d.y, d.x, d.y + e * neck_length);
		c->set_line_width(NOTE_BAR_WIDTH);
		c->draw_line(d.x, d.y + e * neck_length, d.x + NOTE_FLAG_DX, d.y + e * (neck_length - NOTE_FLAG_DY));
	}else if (d.base_length == QUARTER){
		c->set_line_width(neck_width);
		c->draw_line(d.x, d.y, d.x, d.y + e * neck_length);
	}

	if (d.punctured)
		c->draw_circle(d.x + rr, d.y + rr, 2);
	if (d.triplet)
		c->draw_str(d.x, d.y + e*neck_length + e * 9 - 4, "3");
}

void draw_group_ndata(Painter *c, const Array<NoteData> &d, float rr)
{
	float neck_length = max(NOTE_NECK_LENGTH, rr*5);
	float neck_width = max(NOTE_NECK_WIDTH, rr/3);
	float e = d[0].up ? -1 : 1;
	float x0 = d[0].x;
	float y0 = d[0].y;
	float x1 = d.back().x;
	float y1 = d.back().y;
	float dx = x1 - x0;
	float dy = y1 - y0;
	float m = dy / dx;

	// optimize neck length
	float dy0min = 0;
	for (auto &dd: d)
		dy0min = max(dy0min, e * (dd.y - (y0 + m * (dd.x - x0))));
	y0 += e * max(neck_length, dy0min + neck_length * 0.6f);

	// draw necks
	c->set_line_width(neck_width);
	for (auto &dd: d)
		if (dd.n){
			c->set_color(dd.col);
			c->draw_line(dd.x, dd.y, dd.x, y0 + m * (dd.x - x0));
		}


	// bar
	c->set_line_width(NOTE_BAR_WIDTH);
	float t0 = 0;
	for (int i=0; i<d.num; i++){
		float xx = d[i].x;
		if (i+1 < d.num){
			if (d[i+1].length == d[i].length)
				xx = (d[i+1].x + d[i].x)/2;
				//xx = (d[i+1].x * d[i].length + d[i].x * d[i+1].length)/(d[i].length + d[i+1].length);
			if (d[i+1].length < d[i].length)
				xx = d[i+1].x;
		}
		if (xx == x0)
			xx = (x0*2 + d[1].x) / 3;
		if (xx == x1 and (i+1 < d.num))
			xx = (x1*4 + d[1-1].x) / 5;
		float t1 = (xx - x0) / dx;
		if (d[i].n){
			c->set_color(d[i].col);
			c->draw_line(x0 + dx*t0, y0 + dy*t0, x0 + dx*t1, y0 + dy*t1);
			if (d[i].length <= SIXTEENTH)
				c->draw_line(x0 + dx*t0, y0 + dy*t0 - e*NOTE_BAR_DISTANCE, x0 + dx*t1, y0 + dy*t1 - e*NOTE_BAR_DISTANCE);
			if (d[i].punctured)
				c->draw_circle(d[i].x + rr, d[i].y + rr, 2);
		}
		t0 = t1;
	}
	if (d[0].triplet)
		//c->draw_str((x0 + x1)/2, (y0 + y1)/2 - 4 + e*9, "3");
		c->draw_str(x0 + dx/2, y0 + dy/2 + c->font_size * (e*1.3f - 0.5f), "3");
}


bool find_group(Array<NoteData> &ndata, NoteData &d, int i, Array<NoteData> &group, int beat_length)
{
	group = {d};
	for (int j=i+1; j<ndata.num; j++){
		// non-continguous?
		if (ndata[j].offset != ndata[j-1].end)
			break;
		if (ndata[j].triplet != d.triplet)
			break;
		// size mismatch?
		if ((ndata[j].length != 4) and (ndata[j].length != 2) and (ndata[j].length != 9) and (ndata[j].length != 6) and (ndata[j].length != 3))
			break;
		// beat finished?
		if (ndata[j].end > d.offset + beat_length)
			break;
		group.add(ndata[j]);
	}
	return (group.back().end - d.offset) == beat_length;
}

// currently only for 4x 8th notes
bool find_long_group(Array<NoteData> &ndata, NoteData &d, int i, Array<NoteData> &group)
{
	if (ndata.num < i + 4)
		return false;
	group = {d};
	for (int j=i+1; j<i+4; j++){
		// non-continguous?
		if (ndata[j].offset != ndata[j-1].end)
			return false;
		// size mismatch?
		if (ndata[j].length != EIGHTH)
			return false;
		group.add(ndata[j]);
	}
	return true;
}

struct QuantizedBar
{
	Array<int> beat_offset;
	Array<int> beat_length;
	QuantizedBar(Bar *b)
	{
		int off = 0;
		for (int bb: b->beats){
			int d = bb * QUARTER_PARTITION / b->divisor;
			beat_length.add(d);
			beat_offset.add(off);
			off += d;
		}
	}
	int get_beat(int offset)
	{
		int _bo = 0;
		for (int i=0; i<beat_offset.num; i++){
			if (offset < beat_offset[i] + beat_length[i]){
				return i;
			}
		}
		return -1;
	}
	int get_rel(int offset, int beat)
	{
		if (beat < 0)
			return -1;
		return offset - beat_offset[beat];
	}
	bool start_of_beat(int offset, int beat)
	{
		return get_rel(offset, beat) == 0;
	}
};


void MidiPainter::draw_rhythm(Painter *c, const MidiNoteBufferRef &midi, const Range &range, std::function<float(MidiNote*)> y_func)
{
	if (cam->scale * song->sample_rate < quality.dx_min)
		return;


	c->set_antialiasing(quality.antialiasing);

	/*if (vlayer->is_playable())
		c->set_color(colors.text_soft1);
	else
		c->set_color(colors.text_soft3);*/

	float y_mid = (area.y1 + area.y2) / 2;

	auto bars = song->bars.get_bars(range);
	for (auto *b: bars){
		if (b->is_pause())
			continue;

		// samples per 16th / 3
		float sub_beat_length = (float)b->range().length / (float)b->total_sub_beats;
		float quarter_length = sub_beat_length * b->divisor;
		float spu = quarter_length / (float)QUARTER_PARTITION;
//		float spu = (float)b->range().length / (float)b->num_beats / (float)BEAT_PARTITION;

		MidiNoteBufferRef bnotes = midi.get_notes(b->range());
		//c->set_color(colors.text_soft3);

		Array<NoteData> ndata;
		for (MidiNote *n: bnotes){
			Range r = n->range - b->offset;
			if (r.offset < 0)
				continue;
			NoteData d = NoteData(n, r, spu);
			if (d.length == 0 or d.offset < 0)
				continue;

			d.x = cam->sample2screen(n->range.offset + shift);
			d.y = y_func(n);
			d.up = (d.y >= y_mid);


			color col_shadow;
			get_col(d.col, col_shadow, n, note_state(n, as_reference, sel, hover), is_playable, colors);

			// prevent double notes
			if (ndata.num > 0)
				if (ndata.back().offset == d.offset){
					// use higher note...
					if (d.n->pitch > ndata.back().n->pitch)
						ndata[ndata.num - 1] = d;
					continue;
				}

			ndata.add(d);
		}

		QuantizedBar qbar(b);
		bool bar_allows_long_groups = b->is_uniform() and (b->beats[0] == b->divisor) and (b->beats.num % 2 == 0);

		//int offset = 0;
		for (int i=0; i<ndata.num; i++){
			NoteData &d = ndata[i];

			// group start?
			int beat_index = qbar.get_beat(d.offset);
			if (qbar.start_of_beat(d.offset, beat_index)){
				if ((d.length == 9 or d.length == 6 or d.length == 3 or d.length == 4 or d.length == 2)){

					Array<NoteData> group;
					bool ok = false;
					bool allow_long_group = false;
					if (bar_allows_long_groups)
						allow_long_group = ((d.offset % (QUARTER_PARTITION*2)) == 0) and (d.length == EIGHTH);
					if (allow_long_group)
						ok = find_long_group(ndata, d, i, group);
					if (!ok)
						ok = find_group(ndata, d, i, group, qbar.beat_length[beat_index]);

					// draw
					if (ok){
						draw_group_ndata(c, group, rr);
						i += group.num - 1;
						continue;
					}
				}
			}

			draw_single_ndata(c, d, rr);
		}

	}
	c->set_line_width(1);
	c->set_antialiasing(false);
}



void MidiPainter::draw_pitch_grid(Painter *c, Synthesizer *synth)
{
	// pitch grid
	c->set_color(color(0.25f, 0, 0, 0));
	for (int i=pitch_min; i<pitch_max; i++){
		float y0 = pitch2y_linear(i + 1);
		float y1 = pitch2y_linear(i);
		if (!midi_scale.contains(i)){
			c->set_color(color(0.2f, 0, 0, 0));
			c->draw_rect(area.x1, y0, area.width(), y1 - y0);
		}
	}


	// pitch names
	color cc = colors.text;
	cc.a = 0.4f;
	Array<SampleRef*> *p = nullptr;
	if ((synth) and (synth->module_subtype == "Sample")){
		auto *c = synth->get_config();
		p = (Array<SampleRef*> *)&c[1];
	}
	bool is_drum = (instrument->type == Instrument::Type::DRUMS);
	float dy = ((pitch2y_linear(0) - pitch2y_linear(1)) - c->font_size) / 2;
	for (int i=pitch_min; i<pitch_max; i++){
		c->set_color(cc);
		if (((hover->type == HoverData::Type::MIDI_PITCH) or (hover->type == HoverData::Type::MIDI_NOTE)) and (i == hover->pitch))
			c->set_color(colors.text);

		string name = pitch_name(i);
		if (is_drum){
			name = drum_pitch_name(i);
		}else if (p){
			if (i < p->num)
				if ((*p)[i])
					name = (*p)[i]->origin->name;
		}
		c->draw_str(20, pitch2y_linear(i+1)+dy, name);
	}
}

void MidiPainter::draw_complex_note(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y)
{
	if (state & MidiPainter::STATE_SELECTED){
		color col1 = colors.selection;
		draw_simple_note(c, x1, x2, y, 2, col1, col1, false);
	}

	color col, col_shadow;
	get_col(col, col_shadow, n, state, is_playable, colors);

	draw_simple_note(c, x1, x2, y, 0, col, col_shadow, false);

	if (n->flags > 0){
		if (n->is(NOTE_FLAG_TRILL))
			c->draw_str(x1, y - rr*3, "tr~");
		if (n->is(NOTE_FLAG_STACCATO))
			c->draw_circle(x1+rr, y + rr*2, 2);
		if (n->is(NOTE_FLAG_TENUTO)){
			c->set_line_width(3);
			c->draw_line(x1, y + rr*2, x1+20, y + rr*2);
			c->set_line_width(1);
		}
	}

}


void MidiPainter::draw_note_linear(Painter *c, const MidiNote &n, MidiNoteState state)
{
	float x1, x2;
	cam->range2screen(n.range, x1, x2);
	float y1 = pitch2y_linear(n.pitch + 1);
	float y2 = pitch2y_linear(n.pitch);

	float y = (y1 + y2) / 2;
	n.y = y;

	draw_complex_note(c, &n, state, x1, x2, y);
}

void MidiPainter::draw_linear(Painter *c, const MidiNoteBufferRef &notes)
{
	//c->setLineWidth(3.0f);

/*	if (view->mode_midi->editing(this)){
		pitch_min = edit_pitch_min;
		pitch_max = edit_pitch_max;
	}else{
		pitch_min = PITCH_MIN_DEFAULT;
		pitch_max = PITCH_MAX_DEFAULT;
	}*/

	// draw notes
	c->set_antialiasing(quality.antialiasing);
	for (MidiNote *n: notes){
		if ((n->pitch < pitch_min) or (n->pitch >= pitch_max))
			continue;
		draw_note_linear(c, *n, note_state(n, as_reference, sel, hover));
	}
	c->set_antialiasing(false);
}

void MidiPainter::draw_simple_note(Painter *c, float x1, float x2, float y, float rx, const color &col, const color &col_shadow, bool force_circle)
{
	//x1 += r;
	// "shadow" to indicate length
	if (x2 - x1 > quality.shadow_threshold){
		c->set_color(col_shadow);
		c->draw_rect(x1, y - rr*0.7f - rx, x2 - x1 + rx, rr*2*0.7f + rx*2);
	}

	// the note circle
	c->set_color(col);
	if ((x2 - x1 > quality.note_circle_threshold) or force_circle)
		c->draw_circle(x1, y, rr+rx);
	else
		c->draw_rect(x1 - rr*0.8f - rx, y - rr*0.8f - rx, rr*1.6f + rx*2, rr*1.6f + rx*2);
}

void MidiPainter::draw_clef_tab(Painter *c)
{
	if (is_playable)
		c->set_color(colors.text_soft1);
	else
		c->set_color(colors.text_soft3);

	// clef lines
	float h = string_dy * instrument->string_pitch.num;
	for (int i=0; i<instrument->string_pitch.num; i++){
		float y = string_to_screen(i);
		c->draw_line(area.x1, y, area.x2, y);
	}


	if (is_playable)
		c->set_color(colors.text);
	else
		c->set_color(colors.text_soft3);

	c->set_font_size(h / 6);
	c->draw_str(10, area.y1 + area.height() / 2 - h * 0.37f, "T\nA\nB");
	c->set_font_size(AudioView::FONT_SIZE);
}

void MidiPainter::draw_note_tab(Painter *c, const MidiNote *n, MidiNoteState state)
{
	float x1, x2;
	cam->range2screen(n->range + shift, x1, x2);


	int p = n->stringno;
	float y = string_to_screen(p);
	n->y = y;

	draw_complex_note(c, n, state, x1, x2, y);

	if (x2 - x1 > quality.tab_text_threshold and rr > 5){
		float font_size = rr * 1.2f;
		c->set_color(colors.high_contrast_b);//text);
		SymbolRenderer::draw(c, x1, y - font_size/2, font_size, i2s(n->pitch - instrument->string_pitch[n->stringno]), 0);
	}
}

void MidiPainter::draw_tab(Painter *c, const MidiNoteBufferRef &notes)
{
	draw_rhythm(c, notes, cur_range, [=](MidiNote *n){ return string_to_screen(n->stringno); });

	c->set_antialiasing(quality.antialiasing);
	for (MidiNote *n: notes)
		draw_note_tab(c,  n,  note_state(n, as_reference, sel, hover));
	c->set_antialiasing(false);

	c->set_font_size(AudioView::FONT_SIZE);
}

void MidiPainter::draw_note_classical(Painter *c, const MidiNote *n, MidiNoteState state)
{
	float x1, x2;
	cam->range2screen(n->range + shift, x1, x2);

	// checked before...
//	if (n.clef_position < 0)
//		n.update_meta(track->instrument, midi_scale, 0);

	int p = n->clef_position;
	float y = clef_pos_to_screen(p);
	n->y = y;

	// auxiliary lines
	for (int i=10; i<=p; i+=2){
		c->set_color(colors.text_soft2);
		float y = clef_pos_to_screen(i);
		c->draw_line(x1 - clef_dy, y, x1 + clef_dy, y);
	}
	for (int i=-2; i>=p; i-=2){
		c->set_color(colors.text_soft2);
		float y = clef_pos_to_screen(i);
		c->draw_line(x1 - clef_dy, y, x1 + clef_dy, y);
	}

	draw_complex_note(c, n, state, x1, x2, y);

	if ((n->modifier != NoteModifier::NONE) and (rr >= 3)){
		c->set_color(colors.text);
		//c->setColor(ColorInterpolate(col, colors.text, 0.5f));
		float size = rr*2.8f;
		SymbolRenderer::draw(c, x1 - size*0.7f, y - size*0.5f , size, modifier_symbol(n->modifier));
	}
}

void MidiPainter::draw_key_symbol(Painter *c, const MidiKeyChange &kc)
{
	float x = cam->sample2screen(kc.pos);

	c->set_font_size(clef_dy*4);
	c->draw_str(x + 10, clef_pos_to_screen(8), clef->symbol);
	c->set_font_size(clef_dy);

	for (int i=0; i<7; i++){
		if (kc.key.modifiers[i] != NoteModifier::NONE)
			c->draw_str(x + 18 + clef_dy*3.0f + clef_dy*0.6f*(i % 3), clef_pos_to_screen((i - clef->offset + 7*20) % 7) - clef_dy*0.5f, modifier_symbol(kc.key.modifiers[i]));
	}
	c->set_font_size(AudioView::FONT_SIZE);
}

void MidiPainter::draw_clef_classical(Painter *c)
{
	// clef lines

	if (is_playable)
		c->set_color(colors.text_soft1);
	else
		c->set_color(colors.text_soft3);

	for (int i=0; i<10; i+=2){
		float y = clef_pos_to_screen(i);
		c->draw_line(area.x1, y, area.x2, y);
	}

	if (is_playable)
		c->set_color(colors.text);
	else
		c->set_color(colors.text_soft3);

	// clef symbol

	Scale key_prev = Scale::C_MAJOR;
	for (auto &kc: key_changes)
		if (kc.pos < cam->range().offset)
			key_prev = kc.key;
	draw_key_symbol(c, MidiKeyChange(cam->range().offset, key_prev));

	for (auto &kc: key_changes)
		draw_key_symbol(c, kc);

	c->set_font_size(AudioView::FONT_SIZE);
}



void MidiPainter::draw_classical(Painter *c, const MidiNoteBufferRef &notes)
{
	draw_rhythm(c, notes, cur_range, [=](MidiNote *n){ return clef_pos_to_screen(n->clef_position); });

	c->set_antialiasing(quality.antialiasing);
	for (MidiNote *n: notes)
		draw_note_classical(c, n, note_state(n, as_reference, sel, hover));
	c->set_antialiasing(false);

	c->set_font_size(AudioView::FONT_SIZE);
}

void MidiPainter::draw_low_detail_dummy(Painter *c, const MidiNoteBufferRef &notes)
{
	auto bars = song->bars.get_bars(cur_range);
	for (auto *b: bars){
		float x0, x1;
		cam->range2screen(b->range(), x0, x1);
		MidiNoteBufferRef bnotes = notes.get_notes(b->range());
		int count[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
		for (MidiNote *n: bnotes)
			count[pitch_to_rel(n->pitch)] ++;
		for (int i=0; i<12; i++){
			if (count[i] == 0)
				continue;
			color col = pitch_color(i);
			if (!is_playable)
				col = ColorInterpolate(col, colors.text_soft3, 0.8f);
			col.a = (float)count[i] / (float)bnotes.num;
			c->set_color(col);
			float y0 = area.y2 - i*area.height()/12;
			float y1 = area.y2 - (i+1)*area.height()/12;
			c->draw_rect(rect(x0, x1, y0, y1));
		}
	}
}

Range extend_range_to_bars(const Range &r, const BarCollection &bars)
{
	Range rr = r;
	for (auto &b: bars){
		if (b->range().is_more_inside(rr.start()))
			rr.set_start(b->range().start());
		if (b->range().is_more_inside(rr.end()))
			rr.set_end(b->range().end());
	}
	return rr;
}

void MidiPainter::draw(Painter *c, const MidiNoteBuffer &midi)
{
	cur_range = extend_range_to_bars(cam->range() - shift, song->bars);
	midi.update_clef_pos(*instrument, midi_scale);
	MidiNoteBufferRef notes = midi.get_notes(cur_range);

	float w_1min = cam->dsample2screen(song->sample_rate * 60);
	if (notes.num > quality.note_count_threshold or (w_1min < 1000/quality.factor)){
		draw_low_detail_dummy(c, notes);
		return;
	}
	
	if (mode == MidiMode::LINEAR)
		draw_linear(c, notes);
	else if (mode == MidiMode::TAB)
		draw_tab(c, notes);
	else // if (mode == MidiMode::CLASSICAL)
		draw_classical(c, notes);
}

void MidiPainter::draw_background(Painter *c)
{
	if (mode == MidiMode::CLASSICAL){
		draw_clef_classical(c);
	}else if (mode == MidiMode::TAB){
		draw_clef_tab(c);
	}
}

void MidiPainter::set_context(const rect& _area, const Instrument& i, bool _is_playable, MidiMode _mode)
{
	area = _area;
	instrument = &i;
	clef = &i.get_clef();

	// TAB
	string_dy = min((area.height() * 0.7f) / instrument->string_pitch.num, 40.0f);
	float h = string_dy * instrument->string_pitch.num;
	string_y0 = area.y2 - (area.height() - h) / 2 - string_dy/2;

	// classical clef
	clef_dy = min(area.height() / 13, 30.0f);
	clef_y0 = area.y2 - area.height() / 2 + 2 * clef_dy;

	is_playable = _is_playable;
	as_reference = false;
	shift = 0;
	mode = _mode;

	pitch_min = PITCH_MIN_DEFAULT;
	pitch_max = PITCH_MAX_DEFAULT;

	if (mode == MidiMode::CLASSICAL)
		rr = min(clef_dy/2, 10.0f);
	if (mode == MidiMode::LINEAR)
		rr = max((pitch2y_linear(0) - pitch2y_linear(1)) / 2.3f, 2.0f);
	if (mode == MidiMode::TAB)
		rr = min(string_dy/2, 13.0f);
}

void MidiPainter::set_key_changes(const Array<MidiKeyChange> &changes)
{
	key_changes = changes;
}

void MidiPainter::set_linear_range(int _pitch_min, int _pitch_max)
{
	pitch_min = _pitch_min;
	pitch_max = _pitch_max;
}

void MidiPainter::set_shift(int _shift)
{
	shift = _shift;
}

void MidiPainter::set_quality(float q, bool antialiasing)
{
	quality.factor = q;
	quality.dx_min = 20 / q;
	quality.shadow_threshold = rr*1.5f / q;
	quality.note_circle_threshold = 6 / q;
	quality.tab_text_threshold = rr/4 / q;
	quality.antialiasing = antialiasing;
	quality.note_count_threshold = area.width() * 0.4f * q;
}
