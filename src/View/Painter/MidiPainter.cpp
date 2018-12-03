/*
 * MidiPainter.cpp
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#include "MidiPainter.h"
#include "../AudioView.h"
#include "../ViewPort.h"
#include "../AudioViewLayer.h" // argh
#include "../../Data/Song.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/Midi/Clef.h"
#include "../../Data/Midi/Instrument.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/SampleRef.h"
#include "../../Data/Sample.h"
#include "../Helper/SymbolRenderer.h"
#include "../../Module/Synth/Synthesizer.h"


// rhythm quantization
static const int BEAT_PARTITION = 12;
static const int QUARTER = BEAT_PARTITION;
static const int EIGHTH = BEAT_PARTITION / 2;
static const int SIXTEENTH = BEAT_PARTITION / 4;
static const int TRIPLET_EIGHTH = BEAT_PARTITION / 3;
static const int TRIPLET_SIXTEENTH = BEAT_PARTITION / 6;

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

MidiPainter::MidiPainter(AudioView *_view)
{
	view = _view;
	song = view->song;
	instrument = nullptr;
	scale = nullptr;
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
}


color MidiPainter::pitch_color(int pitch)
{
	return PITCH_COLORS[pitch % 12];
	//return SetColorHSB(1, (float)(pitch % 12) / 12.0f, 0.6f, 1);
}


void get_col(color &col, color &col_shadow, const MidiNote *n, MidiPainter::MidiNoteState state, bool playable)
{
	if (playable)
		col = ColorInterpolate(MidiPainter::pitch_color(n->pitch), AudioView::colors.text, 0.2f);
	else
		col = AudioView::colors.text_soft3;

	if (state & MidiPainter::STATE_HOVER){
		col = ColorInterpolate(col, AudioView::colors.hover, 0.333f);
	}else if (state & MidiPainter::STATE_REFERENCE){
		col = ColorInterpolate(col, AudioView::colors.background_track, 0.65f);
	}
	col_shadow = ColorInterpolate(col, AudioView::colors.background_track, 0.3f);
}


inline MidiPainter::MidiNoteState note_state(MidiNote *n, bool as_reference, AudioView *view)
{
	MidiPainter::MidiNoteState s = MidiPainter::STATE_DEFAULT;
	if (view->sel.has(n))
		s = MidiPainter::STATE_SELECTED;
	if (as_reference)
		return (MidiPainter::MidiNoteState)(MidiPainter::STATE_REFERENCE | s);
	if ((view->hover.type == Selection::Type::MIDI_NOTE) and (n == view->hover.note))
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
	int p = clef->pitch_to_position(pitch, view->midi_scale, mod);
	return clef_pos_to_screen(p);
}

int MidiPainter::y2pitch_linear(float y)
{
	return pitch_min + ((area.y2 - y) * (pitch_max - pitch_min) / area.height());
}

int MidiPainter::y2pitch_classical(float y, NoteModifier modifier)
{
	int pos = screen_to_clef_pos(y);
	return clef->position_to_pitch(pos, view->midi_scale, modifier);
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
	return clef->pitch_to_position(pitch, view->midi_scale, mod);
}

struct NoteData
{
	NoteData(){}
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
	c->set_color(d.col);
	float e = d.up ? -1 : 1;
	if (d.base_length == SIXTEENTH){
		c->set_line_width(NOTE_NECK_WIDTH);
		c->draw_line(d.x, d.y, d.x, d.y + e * neck_length);
		c->set_line_width(NOTE_BAR_WIDTH);
		c->draw_line(d.x, d.y + e * neck_length, d.x + NOTE_FLAG_DX, d.y + e * (neck_length - NOTE_FLAG_DY));
		c->draw_line(d.x, d.y + e * (neck_length - NOTE_BAR_DISTANCE), d.x + NOTE_FLAG_DX, d.y + e * (neck_length - NOTE_BAR_DISTANCE - NOTE_FLAG_DY));
	}else if (d.base_length == EIGHTH){
		c->set_line_width(NOTE_NECK_WIDTH);
		c->draw_line(d.x, d.y, d.x, d.y + e * neck_length);
		c->set_line_width(NOTE_BAR_WIDTH);
		c->draw_line(d.x, d.y + e * neck_length, d.x + NOTE_FLAG_DX, d.y + e * (neck_length - NOTE_FLAG_DY));
	}else if (d.base_length == QUARTER){
		c->set_line_width(NOTE_NECK_WIDTH);
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
	float e = d[0].up ? -1 : 1;
	float x0 = d[0].x, y0 = d[0].y + e * neck_length;
	float x1 = d.back().x, y1 = d.back().y + e * neck_length;
	float m = (y1 - y0) / (x1 - x0);

	c->set_line_width(NOTE_NECK_WIDTH);
	for (auto &dd: d){
		c->set_color(dd.col);
		c->draw_line(dd.x, dd.y, dd.x, y0 + m * (dd.x - x0));
	}
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
		float t1 = (xx - x0) / (x1 - x0);
		c->set_color(d[i].col);
		c->draw_line(x0 + (x1-x0)*t0, y0 + (y1-y0)*t0, x0 + (x1-x0)*t1, y0 + (y1-y0)*t1);
		if (d[i].length <= SIXTEENTH)
			c->draw_line(x0 + (x1-x0)*t0, y0 + (y1-y0)*t0 - e*NOTE_BAR_DISTANCE, x0 + (x1-x0)*t1, y0 + (y1-y0)*t1 - e*NOTE_BAR_DISTANCE);
		if (d[i].punctured)
			c->draw_circle(d[i].x + rr, d[i].y + rr, 2);
		t0 = t1;
	}
	if (d[0].triplet)
		c->draw_str((x0 + x1)/2, (y0 + y1)/2 - 4 + e*9, "3");
}


bool find_group(Array<NoteData> &ndata, NoteData &d, int i, Array<NoteData> &group)
{
	group = d;
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
		if (ndata[j].end > d.offset + BEAT_PARTITION)
			break;
		group.add(ndata[j]);
	}
	return (group.back().end - d.offset) == BEAT_PARTITION;
}

bool find_long_group(Array<NoteData> &ndata, NoteData &d, int i, Array<NoteData> &group)
{
	if (ndata.num < i + 4)
		return false;
	group = d;
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

void MidiPainter::draw_rhythm(Painter *c, const MidiNoteBuffer &midi, const Range &range, std::function<float(MidiNote*)> y_func)
{
	if (view->cam.scale * song->sample_rate < 20)
		return;

	/*if (vlayer->is_playable())
		c->set_color(view->colors.text_soft1);
	else
		c->set_color(view->colors.text_soft3);*/

	float y_mid = (area.y1 + area.y2) / 2;

	auto bars = song->bars.get_bars(range);
	for (auto *b: bars){

		// samples per 16th / 3
		float spu = (float)b->range().length / (float)b->num_beats / (float)BEAT_PARTITION;

		MidiNoteBufferRef bnotes = midi.get_notes(b->range());
		//c->set_color(view->colors.text_soft3);

		Array<NoteData> ndata;
		for (MidiNote *n: bnotes){
			Range r = n->range - b->offset;
			if (r.offset < 0)
				continue;
			NoteData d = NoteData(n, r, spu);
			if (d.length == 0 or d.offset < 0)
				continue;

			d.x = view->cam.sample2screen(n->range.offset + shift);
			d.y = y_func(n);
			d.up = (d.y >= y_mid);


			color col_shadow;
			get_col(d.col, col_shadow, n, note_state(n, as_reference, view), is_playable);

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

		//int offset = 0;
		for (int i=0; i<ndata.num; i++){
			NoteData &d = ndata[i];

			// group start?
			if ((d.offset % BEAT_PARTITION) == 0){ // only on start of beat
				if ((d.length == 9 or d.length == 6 or d.length == 3 or d.length == 4 or d.length == 2)){

					Array<NoteData> group;
					bool ok = false;
					bool allow_long_group = ((d.offset % (BEAT_PARTITION*2)) == 0) and (d.length == EIGHTH);
					if (allow_long_group)
						ok = find_long_group(ndata, d, i, group);
					if (!ok)
						ok = find_group(ndata, d, i, group);

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
}



void MidiPainter::draw_pitch_grid(Painter *c, Synthesizer *synth)
{
	// pitch grid
	c->set_color(color(0.25f, 0, 0, 0));
	for (int i=pitch_min; i<pitch_max; i++){
		float y0 = pitch2y_linear(i + 1);
		float y1 = pitch2y_linear(i);
		if (!view->midi_scale.contains(i)){
			c->set_color(color(0.2f, 0, 0, 0));
			c->draw_rect(area.x1, y0, area.width(), y1 - y0);
		}
	}


	// pitch names
	color cc = view->colors.text;
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
		if (((view->hover.type == Selection::Type::MIDI_PITCH) or (view->hover.type == Selection::Type::MIDI_NOTE)) and (i == view->hover.pitch))
			c->set_color(view->colors.text);

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
		color col1 = view->colors.selection;
		draw_simple_note(c, x1, x2, y, 2, col1, col1, false);
	}

	color col, col_shadow;
	get_col(col, col_shadow, n, state, is_playable);

	draw_simple_note(c, x1, x2, y, 0, col, col_shadow, false);

}


void MidiPainter::draw_note_linear(Painter *c, const MidiNote &n, MidiNoteState state)
{
	float x1, x2;
	view->cam.range2screen(n.range, x1, x2);
	float y1 = pitch2y_linear(n.pitch + 1);
	float y2 = pitch2y_linear(n.pitch);

	float y = (y1 + y2) / 2;
	n.y = y;

	draw_complex_note(c, &n, state, x1, x2, y);
}

void MidiPainter::draw_linear(Painter *c, const MidiNoteBuffer &midi)
{
	Range range = view->cam.range() - shift;
	MidiNoteBufferRef notes = midi.get_notes(range);
	//c->setLineWidth(3.0f);

/*	if (view->mode_midi->editing(this)){
		pitch_min = edit_pitch_min;
		pitch_max = edit_pitch_max;
	}else{
		pitch_min = PITCH_MIN_DEFAULT;
		pitch_max = PITCH_MAX_DEFAULT;
	}*/

	// draw notes
	for (MidiNote *n: midi){
		if ((n->pitch < pitch_min) or (n->pitch >= pitch_max))
			continue;
		draw_note_linear(c, *n, note_state(n, as_reference, view));
	}
}

void MidiPainter::draw_simple_note(Painter *c, float x1, float x2, float y, float rx, const color &col, const color &col_shadow, bool force_circle)
{
	//x1 += r;
	// "shadow" to indicate length
	if (x2 - x1 > rr*1.5f){
		c->set_color(col_shadow);
		c->draw_rect(x1, y - rr*0.7f - rx, x2 - x1 + rx, rr*2*0.7f + rx*2);
	}

	// the note circle
	c->set_color(col);
	if ((x2 - x1 > 6) or force_circle)
		c->draw_circle(x1, y, rr+rx);
	else
		c->draw_rect(x1 - rr*0.8f - rx, y - rr*0.8f - rx, rr*1.6f + rx*2, rr*1.6f + rx*2);
}

void MidiPainter::draw_clef_tab(Painter *c)
{
	if (is_playable)
		c->set_color(view->colors.text_soft1);
	else
		c->set_color(view->colors.text_soft3);

	// clef lines
	float h = string_dy * instrument->string_pitch.num;
	for (int i=0; i<instrument->string_pitch.num; i++){
		float y = string_to_screen(i);
		c->draw_line(area.x1, y, area.x2, y);
	}
	c->set_antialiasing(true);


	if (is_playable)
		c->set_color(view->colors.text);
	else
		c->set_color(view->colors.text_soft3);

	c->set_font_size(h / 6);
	c->draw_str(10, area.y1 + area.height() / 2 - h * 0.37f, "T\nA\nB");
	c->set_font_size(view->FONT_SIZE);
}

void MidiPainter::draw_note_tab(Painter *c, const MidiNote *n, MidiNoteState state)
{
	float x1, x2;
	view->cam.range2screen(n->range + shift, x1, x2);


	int p = n->stringno;
	float y = string_to_screen(p);
	n->y = y;

	draw_complex_note(c, n, state, x1, x2, y);

	if (x2 - x1 > rr/4 and rr > 5){
		float font_size = rr * 1.2f;
		c->set_color(view->colors.high_contrast_b);//text);
		SymbolRenderer::draw(c, x1, y - font_size/2, font_size, i2s(n->pitch - instrument->string_pitch[n->stringno]), 0);
	}
}

void MidiPainter::draw_tab(Painter *c, const MidiNoteBuffer &midi)
{
	Range range = view->cam.range() - shift;
	midi.update_meta(*instrument, view->midi_scale);
	MidiNoteBufferRef notes = midi.get_notes(range);


	draw_rhythm(c, midi, range, [&](MidiNote *n){ return string_to_screen(n->stringno); });

	for (MidiNote *n: notes)
		draw_note_tab(c,  n,  note_state(n, as_reference, view));

	c->set_antialiasing(false);
	c->set_font_size(view->FONT_SIZE);
}

void MidiPainter::draw_note_classical(Painter *c, const MidiNote *n, MidiNoteState state)
{
	float x1, x2;
	view->cam.range2screen(n->range + shift, x1, x2);

	// checked before...
//	if (n.clef_position < 0)
//		n.update_meta(track->instrument, view->midi_scale, 0);

	int p = n->clef_position;
	float y = clef_pos_to_screen(p);
	n->y = y;

	// auxiliary lines
	for (int i=10; i<=p; i+=2){
		c->set_color(view->colors.text_soft2);
		float y = clef_pos_to_screen(i);
		c->draw_line(x1 - clef_dy, y, x1 + clef_dy, y);
	}
	for (int i=-2; i>=p; i-=2){
		c->set_color(view->colors.text_soft2);
		float y = clef_pos_to_screen(i);
		c->draw_line(x1 - clef_dy, y, x1 + clef_dy, y);
	}

	draw_complex_note(c, n, state, x1, x2, y);

	if ((n->modifier != NoteModifier::NONE) and (rr >= 3)){
		c->set_color(view->colors.text);
		//c->setColor(ColorInterpolate(col, view->colors.text, 0.5f));
		float size = rr*2.8f;
		SymbolRenderer::draw(c, x1 - size*0.7f, y - size*0.5f , size, modifier_symbol(n->modifier));
	}
}

void MidiPainter::draw_clef_classical(Painter *c)
{
	// clef lines

	if (is_playable)
		c->set_color(view->colors.text_soft1);
	else
		c->set_color(view->colors.text_soft3);

	for (int i=0; i<10; i+=2){
		float y = clef_pos_to_screen(i);
		c->draw_line(area.x1, y, area.x2, y);
	}

	if (is_playable)
		c->set_color(view->colors.text);
	else
		c->set_color(view->colors.text_soft3);

	// clef symbol
	c->set_font_size(clef_dy*4);
	c->draw_str(area.x1 + 10, clef_pos_to_screen(8), clef->symbol);
	c->set_font_size(clef_dy);

	for (int i=0; i<7; i++)
		if (scale->modifiers[i] != NoteModifier::NONE)
			c->draw_str(area.x1 + 18 + clef_dy*3.0f + clef_dy*0.6f*(i % 3), clef_pos_to_screen((i - clef->offset + 7*20) % 7) - clef_dy*0.5f, modifier_symbol(scale->modifiers[i]));
	c->set_font_size(view->FONT_SIZE);
}



void MidiPainter::draw_classical(Painter *c, const MidiNoteBuffer &midi)
{
	Range range = view->cam.range() - shift;
	midi.update_meta(*instrument, view->midi_scale);
	MidiNoteBufferRef notes = midi.get_notes(range);

	c->set_antialiasing(true);



	draw_rhythm(c, midi, range, [&](MidiNote *n){ return clef_pos_to_screen(n->clef_position); });

	for (MidiNote *n: notes)
		draw_note_classical(c, n, note_state(n, as_reference, view));

	c->set_font_size(view->FONT_SIZE);
	c->set_antialiasing(false);
}

void MidiPainter::draw(Painter *c, const MidiNoteBuffer &midi)
{
	if (mode == MidiMode::LINEAR)
		draw_linear(c, midi);
	else if (mode == MidiMode::TAB)
		draw_tab(c, midi);
	else // if (mode == MidiMode::CLASSICAL)
		draw_classical(c, midi);
}

void MidiPainter::draw_background(Painter *c)
{
	if (mode == MidiMode::CLASSICAL){
		draw_clef_classical(c);
	}else if (mode == MidiMode::TAB){
		draw_clef_tab(c);
	}
}

void MidiPainter::set_context(const rect& _area, const Instrument& i, const Scale &s, bool _is_playable, MidiMode _mode)
{
	area = _area;
	instrument = &i;
	clef = &i.get_clef();
	scale = &s;

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

void MidiPainter::set_linear_range(int _pitch_min, int _pitch_max)
{
	pitch_min = _pitch_min;
	pitch_max = _pitch_max;
}

void MidiPainter::set_shift(int _shift)
{
	shift = _shift;
}
