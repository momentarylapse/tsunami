/*
 * MidiPainter.cpp
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#include "MidiPainter.h"
#include "midi/rhythm.h"
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



MidiKeyChange::MidiKeyChange(double _pos, const Scale &_key) : pos(_pos), key(_key) {}

MidiKeyChange::MidiKeyChange() : MidiKeyChange(0, Scale::C_MAJOR) {}



MidiPainter::MidiPainter(Song *_song, ViewPort *_cam, SongSelection *_sel, HoverData *_hover, ColorScheme &_colors) :
	midi_scale(Scale::C_MAJOR),
	local_theme(_colors),
	mode_classical(this, _song, _cam, _sel, _hover, _colors),
	mode_tab(this, _song, _cam, _sel, _hover, _colors),
	mode_linear(this, _song, _cam, _sel, _hover, _colors),
	mode_dummy(this, _song, _cam, _sel, _hover, _colors)
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
	mmode = &mode_dummy;
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


void get_col(color &col, color &col_shadow, const MidiNote *n, MidiNoteState state, bool playable, ColorScheme &colors) {
	if (playable)
		col = colors.pitch_text[(int)n->pitch % 12];
	else
		col = colors.text_soft3;

	if (state & MidiNoteState::REFERENCE)
		col = color::interpolate(col, colors.background_track, 0.65f);
	if (state & MidiNoteState::SELECTED)
		col = colors.text;//::interpolate(col, colors.selection, 0.5f);
	if (state & MidiNoteState::HOVER)
		col = color::interpolate(col, colors.hover, 0.5f);

	col_shadow = color::interpolate(col, colors.background_track, 0.5f);
	//col_shadow = col;
	//col_shadow.a = 0.5f;
}


MidiNoteState note_state(MidiNote *n, bool as_reference, SongSelection *sel, HoverData *hover) {
	MidiNoteState s = MidiNoteState::DEFAULT;
	if (sel->has(n))
		s = MidiNoteState::SELECTED;
	if (as_reference)
		return MidiNoteState::REFERENCE | s;
	if ((hover->type == HoverData::Type::MIDI_NOTE) and (n == hover->note))
		return MidiNoteState::HOVER | s;
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
	return clamp((int)floor((string_y0 - y) / string_dy + 0.5f), 0, instrument->string_pitch.num-1);
}

// this is the "center" of the note!
// it reaches below and above this y
float MidiPainter::pitch2y_linear(float pitch) {
	return mode_linear.pitch2y(pitch);
}

float MidiPainter::pitch2y_classical(int pitch) {
	//return mode_classical.pitch2y(pitch);
	NoteModifier mod;
	int p = clef->pitch_to_position(pitch, midi_scale, mod);
	return clef_pos_to_screen(p);
}

int MidiPainter::y2pitch_linear(float y) {
	return mode_linear.y2pitch(y);
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
	return mode_linear.y2clef(y, mod);
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



void MidiPainter::draw_rhythm(Painter *c, const MidiNoteBuffer &midi, const Range &range, std::function<float(MidiNote*)> y_func) {

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



void MidiPainter::set_synthesizer(Synthesizer *s) {
	synth = s;
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

	// the note circle
	c->set_color(col);
	if ((x2 - x1 > quality.note_circle_threshold) or force_circle)
		c->draw_circle({(x1+x2)/2, y}, rr+rx);
	else
		c->draw_rect(rect(x1 - rr*0.8f - rx, x1 + rr*0.8f + rx, y - rr*0.8f - rx, y + rr*0.8f + rx));
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
	mmode->draw_notes(p, notes);
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

void MidiPainter::draw_background(Painter *c, bool force) {
	mmode->draw_background(c, force);
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
	mmode = &mode_dummy;
	if (mode == MidiMode::CLASSICAL)
		mmode = &mode_classical;
	else if (mode == MidiMode::TAB)
		mmode = &mode_tab;
	else if (mode == MidiMode::LINEAR)
		mmode = &mode_linear;
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

	quality._highest_details = (cam->pixels_per_sample >= quality.rhythm_zoom_min);
}
void MidiPainter::set_force_shadows(bool force) {
	force_shadows = force;
	allow_shadows = force;// or (mode == MidiMode::LINEAR);
}

float MidiPainter::note_r() const {
	return rr;
}

float MidiPainter::get_clef_dy() const {
	return clef_dy;
}
