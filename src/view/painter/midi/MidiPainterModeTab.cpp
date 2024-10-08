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

namespace tsunami {

MidiPainterModeTab::MidiPainterModeTab(MidiPainter *mp) :
    MidiPainterMode(mp)
{
}

void MidiPainterModeTab::reset() {
}

void MidiPainterModeTab::update() {
	if (direct_size_mode)
		// no padding
		string_dy = mp->area.height() / mp->instrument->string_pitch.num;
	else
		// with padding
		string_dy = mp->area.height() / (max(mp->instrument->string_pitch.num, 7) + 2);
	float h = string_dy * mp->instrument->string_pitch.num;
	string_y0 = mp->area.center().y + (h - string_dy) / 2;

	clef_line_width = mp->area.height() / 200;

	shadow_width = clef_line_width * 9.0f;
	if (local_theme.is_dark())
		shadow_width = clef_line_width * 8.0f;

	rr = string_dy * 0.4f;
	shadow_offset = rr / 4;
	//shadow_hole = 0;
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


	int p = n->stringno;
	float y = string_to_screen(p);
	n->y = y;

	color col, col_shadow;
	get_col(col, col_shadow, n, state, mp->is_playable, local_theme);

	float x = (x1 + x2) / 2;
	float font_size = rr * 1.8f;

	if (n->is(NoteFlag::Dead)) {
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, "x", true, 0);
		return;
	}

	float shadow_len = (x2 - x1) - shadow_offset*2 - shadow_hole;

	if (shadow_len > mp->quality.tab_text_threshold /*and rr > 5*/) {
		string tt = i2s(n->pitch - mp->instrument->string_pitch[n->stringno]);
		float hole = rr * 0.8f * tt.num;

		// "shadow" to indicate length
		if (mp->allow_shadows and (shadow_len > mp->quality.shadow_threshold*1.5f)) {
			//draw_shadow(c, x1, x2, y, 2, rr, col, col_shadow);
			hole += rr * 2.0f;
			draw_shadow2(c, x1, x2, y, hole, 1, col_shadow);
		}

		// hide the string line to make the number more readable
		color cc = local_theme.background_track;
		cc.a = 0.5f;

		c->set_color(cc);
		c->draw_rect(rect(x - hole/2, x + hole/2, y - 2, y + 2));

		// fret number as symbol
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, tt, true, 0);
	} else {
		string tt = i2s(n->pitch - mp->instrument->string_pitch[n->stringno]);
		c->set_color(col);
		SymbolRenderer::draw(c, {x, y - font_size/2}, font_size, tt, true, 0);
	}
}

void MidiPainterModeTab::draw_background(Painter *c, bool force) {

	// clef lines
	if (mp->is_playable)
		c->set_color(local_theme.text_soft2);
	else
		c->set_color(local_theme.text_soft3);
	c->set_line_width(clef_line_width);
	c->set_antialiasing(true);

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

}
