#include "MidiPainterModeTab.h"
#include "../MidiPainter.h"
#include "../../ColorScheme.h"
#include "../../audioview/ViewPort.h"
#include "../../helper/SymbolRenderer.h"
#include "../../../data/midi/Scale.h"
#include "../../../data/midi/MidiData.h"
#include "../../../data/midi/Clef.h"
#include "../../../data/midi/Instrument.h"
#include "../../../lib/image/Painter.h"
#include "../../../lib/math/vec2.h"

void get_col(color &col, color &col_shadow, const MidiNote *n, MidiNoteState state, bool playable, ColorScheme &colors);
MidiNoteState note_state(MidiNote *n, bool as_reference, SongSelection *sel, HoverData *hover);
void draw_shadow(Painter *c, float x1, float x2, float y, float rx, float rr, const color &col);
void draw_shadow2(Painter *c, float x1, float x2, float y, float dx, float clef_line_width, const color &col);


MidiPainterModeTab::MidiPainterModeTab(MidiPainter *mp, Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors) :
    MidiPainterMode(mp, song, cam, sel, hover, colors)
{
}

void MidiPainterModeTab::reset() {
}

void MidiPainterModeTab::update() {
	// TAB
	string_dy = min((mp->area.height() * 0.7f) / max(6, mp->instrument->string_pitch.num), 40.0f);
	float h = string_dy * mp->instrument->string_pitch.num;
	string_y0 = mp->area.y2 - (mp->area.height() - h) / 2 - string_dy/2;

	clef_line_width = mp->area.height() / 150;
}

void MidiPainterModeTab::draw_notes(Painter *c, const MidiNoteBuffer &midi) {
	if (mp->quality._highest_details)
		mp->draw_rhythm(c, midi, mp->cur_range, [this] (MidiNote *n) {
			return string_to_screen(n->stringno);
		});

	c->set_antialiasing(mp->quality.antialiasing);
	for (auto *n: weak(midi))
		draw_note(c,  n,  note_state(n, mp->as_reference, sel, hover));
	c->set_antialiasing(false);

	c->set_font_size(local_theme.FONT_SIZE);
}

void MidiPainterModeTab::draw_note(Painter *c, const MidiNote *n, MidiNoteState state) {
	float x1, x2;
	cam->range2screen(n->range + mp->shift, x1, x2);

    float rr = mp->rr;


	int p = n->stringno;
	float y = string_to_screen(p);
	n->y = y;

	color col, col_shadow;
	get_col(col, col_shadow, n, state, mp->is_playable, local_theme);

	float x = (x1 + x2) / 2;
	float font_size = rr * 1.6f;

	if (n->is(NOTE_FLAG_DEAD)) {
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, "x", true, 0);
		return;
	}

	if (x2 - x1 > mp->quality.tab_text_threshold /*and rr > 5*/) {
		string tt = i2s(n->pitch - mp->instrument->string_pitch[n->stringno]);
		float dx = rr * 0.8f * tt.num;

		// "shadow" to indicate length
		if (mp->allow_shadows and (x2 - x1 > mp->quality.shadow_threshold*1.5f)) {
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

		mp->draw_note_flags(c, n, state, x1, x2, y);
	} else {
		string tt = i2s(n->pitch - mp->instrument->string_pitch[n->stringno]);
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, tt, true, 0);
		mp->draw_note_flags(c, n, state, x1, x2, y);
	}
}

void MidiPainterModeTab::draw_background(Painter *c, bool force) {
	if (mp->is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);
	c->set_line_width(clef_line_width);
	c->set_antialiasing(true);

	// clef lines
	float h = string_dy * mp->instrument->string_pitch.num;
	for (int i=0; i<mp->instrument->string_pitch.num; i++) {
		float y = string_to_screen(i);
		c->draw_line({mp->area.x1, y}, {mp->area.x2, y});
	}
	c->set_antialiasing(false);


	if (mp->is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);

	c->set_font_size(h / 6);
	c->draw_str({10, mp->area.center().y - h * 0.37f}, "T\nA\nB");
	c->set_font_size(local_theme.FONT_SIZE);
}


float MidiPainterModeTab::string_to_screen(int string_no) const {
	return string_y0 - string_no * string_dy;
}

int MidiPainterModeTab::screen_to_string(float y) const {
	return clamp((int)floor((string_y0 - y) / string_dy + 0.5f), 0, mp->instrument->string_pitch.num-1);
}
