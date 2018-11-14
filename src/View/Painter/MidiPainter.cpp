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
}


void get_col(color &col, color &col_shadow, const MidiNote *n, MidiPainter::MidiNoteState state, AudioView *view, bool playable)
{
	if (playable)
		col = ColorInterpolate(AudioViewLayer::pitch_color(n->pitch), view->colors.text, 0.2f);
	else
		col = view->colors.text_soft3;

	if (state & MidiPainter::STATE_HOVER){
		col = ColorInterpolate(col, view->colors.hover, 0.333f);
	}else if (state & MidiPainter::STATE_REFERENCE){
		col = ColorInterpolate(col, view->colors.background_track, 0.65f);
	}
	col_shadow = ColorInterpolate(col, view->colors.background_track, 0.3f);
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

struct NoteData{
	float x, y;
	int offset, length, end;
	MidiNote *n;
	color col;
};

const float NOTE_NECK_LENGTH = 35.0f;
const float NOTE_BAR_DISTANCE = 10.0f;
const float NOTE_BAR_WIDTH = 5.0f;
const float NOTE_NECK_WIDTH = 2.0f;
const float NOTE_FLAG_DX = 10.0f;
const float NOTE_FLAG_DY = 15.0f;

void draw_single_ndata(Painter *c, NoteData &d)
{
	int base_length = d.length;
	bool punctured = ((base_length % 9) == 0);
	if (punctured)
		base_length = (base_length / 3) * 2;
	bool triplets = ((base_length == 2) or (base_length == 4) or (base_length == 8));
	if (triplets)
		base_length = (base_length / 2) * 3;

	c->set_color(d.col);
	if (base_length == 3){
		// 1/16
		c->set_line_width(NOTE_NECK_WIDTH);
		c->draw_line(d.x, d.y, d.x, d.y - NOTE_NECK_LENGTH);
		c->set_line_width(NOTE_BAR_WIDTH);
		c->draw_line(d.x, d.y - NOTE_NECK_LENGTH, d.x + NOTE_FLAG_DX, d.y - NOTE_NECK_LENGTH + NOTE_FLAG_DY);
		c->draw_line(d.x, d.y - NOTE_NECK_LENGTH + NOTE_BAR_DISTANCE, d.x + NOTE_FLAG_DX, d.y - NOTE_NECK_LENGTH + NOTE_BAR_DISTANCE + NOTE_FLAG_DY);
	}else if (base_length == 6){
		// 1/8
		c->set_line_width(NOTE_NECK_WIDTH);
		c->draw_line(d.x, d.y, d.x, d.y - NOTE_NECK_LENGTH);
		c->set_line_width(NOTE_BAR_WIDTH);
		c->draw_line(d.x, d.y - NOTE_NECK_LENGTH, d.x + NOTE_FLAG_DX, d.y - NOTE_NECK_LENGTH + NOTE_FLAG_DY);
	}else if (base_length == 12){
		// 1/4
		c->set_line_width(NOTE_NECK_WIDTH);
		c->draw_line(d.x, d.y, d.x, d.y - NOTE_NECK_LENGTH);
	}

	if (punctured)
		c->draw_circle(d.x + 8, d.y + 5, 2);
	if (triplets)
		c->draw_str(d.x, d.y - NOTE_NECK_LENGTH - 15, "3");
}

void draw_group_ndata(Painter *c, const Array<NoteData> &d)
{
	int base_length = d[0].length;
	bool triplets = ((base_length == 2) or (base_length == 4) or (base_length == 8));
	if (triplets)
		base_length = (base_length / 2) * 3;

	float x0 = d[0].x, y0 = d[0].y-NOTE_NECK_LENGTH;
	float x1 = d.back().x, y1 = d.back().y-NOTE_NECK_LENGTH;
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
		if (d[i].length <= 3)
			c->draw_line(x0 + (x1-x0)*t0, y0 + (y1-y0)*t0 + NOTE_BAR_DISTANCE, x0 + (x1-x0)*t1, y0 + (y1-y0)*t1 + NOTE_BAR_DISTANCE);
		t0 = t1;
	}
	if (triplets)
		c->draw_str((x0 + x1)/2, (y0 + y1)/2 - 17, "3");
}

void MidiPainter::draw_rhythm(Painter *c, const MidiNoteBuffer &midi, const Range &range, std::function<float(MidiNote*)> y_func)
{
	if (view->cam.scale * song->sample_rate < 20)
		return;

	/*if (vlayer->is_playable())
		c->set_color(view->colors.text_soft1);
	else
		c->set_color(view->colors.text_soft3);*/

	auto bars = song->bars.get_bars(range);
	for (auto *b: bars){
		const int BEAT_PARTITION = 12;

		// samples per 16th / 3
		float spu = (float)b->range().length / (float)b->num_beats / (float)BEAT_PARTITION;

		MidiNoteBufferRef bnotes = midi.get_notes(b->range());
		//c->set_color(view->colors.text_soft3);

		Array<NoteData> ndata;
		for (MidiNote *n: bnotes){
			Range r = n->range - b->offset;
			if (r.offset < 0)
				continue;
			NoteData d;
			d.n = n;
			d.offset = int((float)r.offset / spu + 0.5f);
			d.length = int((float)r.end() / spu + 0.5f) - d.offset;
			if (d.length == 0 or d.offset < 0)
				continue;
			d.end = d.offset + d.length;

			d.x = view->cam.sample2screen(n->range.offset + shift);
			d.y = y_func(n);


			color col_shadow;
			get_col(d.col, col_shadow, n, note_state(n, as_reference, view), view, is_playable);

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
				if ((d.length == 6 or d.length == 3 or d.length == 4 or d.length == 2)){
					bool triplet = (d.length == 4) or (d.length == 2);


					Array<NoteData> group = d;
					for (int j=i+1; j<ndata.num; j++){
						// non-continguous?
						if (ndata[j].offset != ndata[j-1].end)
							break;
						// size mismatch?
						if (triplet){
							if (ndata[j].length != 4 and ndata[j].length != 2)
								break;
						}else{
							if ((ndata[j].length != 6) and (ndata[j].length != 3))
								break;
						}
						// beat finished?
						if (ndata[j].end > d.offset + BEAT_PARTITION)
							break;
						group.add(ndata[j]);
					}
					if (group.num > 1){
						if (group.back().end - d.offset == BEAT_PARTITION){
							draw_group_ndata(c, group);
							i += group.num - 1;
							continue;
						}
					}
				}
			}

			draw_single_ndata(c, d);
		}

	}
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
		c->draw_str(20, area.y1 + area.height() * (pitch_max - i - 1) / (pitch_max - pitch_min), name);
	}
}

void MidiPainter::draw_complex_note(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y, float r)
{
	if (state & MidiPainter::STATE_SELECTED){
		color col1 = view->colors.selection;
		draw_simple_note(c, x1, x2, y, r, 2, col1, col1, false);
	}

	color col, col_shadow;
	get_col(col, col_shadow, n, state, view, is_playable);

	draw_simple_note(c, x1, x2, y, r, 0, col, col_shadow, false);

}


void MidiPainter::draw_note_linear(Painter *c, const MidiNote &n, MidiNoteState state)
{
	float x1, x2;
	view->cam.range2screen(n.range, x1, x2);
	float y1 = pitch2y_linear(n.pitch + 1);
	float y2 = pitch2y_linear(n.pitch);

	float y = (y1 + y2) / 2;
	n.y = y;
	float r = max((y2 - y1) / 2.3f, 2.0f);

	draw_complex_note(c, &n, state, x1, x2, y, r);
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

void MidiPainter::draw_simple_note(Painter *c, float x1, float x2, float y, float r, float rx, const color &col, const color &col_shadow, bool force_circle)
{
	//x1 += r;
	// "shadow" to indicate length
	if (x2 - x1 > r*1.5f){
		c->set_color(col_shadow);
		c->draw_rect(x1, y - r*0.7f - rx, x2 - x1 + rx, r*2*0.7f + rx*2);
	}

	// the note circle
	c->set_color(col);
	if ((x2 - x1 > 6) or force_circle)
		c->draw_circle(x1, y, r+rx);
	else
		c->draw_rect(x1 - r*0.8f - rx, y - r*0.8f - rx, r*1.6f + rx*2, r*1.6f + rx*2);
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
	float r = min(string_dy/2, 13.0f);

	float x1, x2;
	view->cam.range2screen(n->range + shift, x1, x2);


	int p = n->stringno;
	float y = string_to_screen(p);
	n->y = y;

	draw_complex_note(c, n, state, x1, x2, y, r);

	if (x2 - x1 > r/4 and r > 5){
		float font_size = r * 1.2f;
		c->set_color(view->colors.high_contrast_b);//text);
		SymbolRenderer::draw(c, x1, y - font_size*0.75f, font_size, i2s(n->pitch - instrument->string_pitch[n->stringno]), 0);
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
	float r = min(clef_dy/2, 10.0f);

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

	draw_complex_note(c, n, state, x1, x2, y, r);

	if ((n->modifier != NoteModifier::NONE) and (r >= 3)){
		c->set_color(view->colors.text);
		//c->setColor(ColorInterpolate(col, view->colors.text, 0.5f));
		float size = r*2.8f;
		SymbolRenderer::draw(c, x1 - size*0.7f, y - size*0.8f , size, modifier_symbol(n->modifier));
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
	c->draw_str(area.x1 + 10, clef_pos_to_screen(10), clef->symbol);
	c->set_font_size(clef_dy);

	for (int i=0; i<7; i++)
		if (scale->modifiers[i] != NoteModifier::NONE)
			c->draw_str(area.x1 + 18 + clef_dy*3.0f + clef_dy*0.6f*(i % 3), clef_pos_to_screen((i - clef->offset + 7*20) % 7) - clef_dy*0.8f, modifier_symbol(scale->modifiers[i]));
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
