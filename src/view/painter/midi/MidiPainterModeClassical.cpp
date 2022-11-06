#include "MidiPainterModeClassical.h"
#include "../MidiPainter.h"
#include "../../ColorScheme.h"
#include "../../audioview/ViewPort.h"
#include "../../helper/SymbolRenderer.h"
#include "../../../data/midi/Scale.h"
#include "../../../data/midi/MidiData.h"
#include "../../../data/midi/Clef.h"
#include "../../../lib/image/Painter.h"
#include "../../../lib/math/vec2.h"


MidiPainterModeClassical::MidiPainterModeClassical(MidiPainter *mp) :
    MidiPainterMode(mp)
{
}

void MidiPainterModeClassical::reset() {
	update();
}

void MidiPainterModeClassical::update() {
	if (direct_size_mode)
		clef_dy = mp->area.height() / 4;
	else
		clef_dy = min(mp->area.height() / 13, 30.0f);
	clef_y0 = mp->area.center().y + 2 * clef_dy;

	clef_line_width = mp->area.height() / 150;

	rr = clef_dy * 0.42f;
}

void MidiPainterModeClassical::draw_notes(Painter *c, const MidiNoteBuffer &midi) {
	if (mp->quality._highest_details)
		mp->draw_rhythm(c, midi, mp->cur_range, [this] (MidiNote *n) {
			return clef_pos_to_screen(n->clef_position);
		});

	c->set_antialiasing(mp->quality.antialiasing);
	for (auto *n: weak(midi))
		draw_note(c, n, note_state(n, mp->as_reference, sel, hover));
	c->set_antialiasing(false);

	c->set_font_size(local_theme.FONT_SIZE);
}



void MidiPainterModeClassical::draw_note(Painter *c, const MidiNote *n, MidiNoteState state) {
	float x1, x2;
	cam->range2screen(n->range + mp->shift, x1, x2);
	float x = (x1 + x2) / 2;

	// checked before...
//	if (n.clef_position <= UNDEFINED_CLEF)
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

	color col, col_shadow;
	get_col(col, col_shadow, n, state, mp->is_playable, local_theme);

	if (state & MidiNoteState::SELECTED) {
		color col1 = local_theme.pitch_text[(int)n->pitch % 12];//selection;

		// "shadow" to indicate length
		if (mp->allow_shadows and (x2 - x1 > mp->quality.shadow_threshold)) {
			//draw_shadow(c, x1, x2, y, rx, rr, col_shadow);
			draw_shadow2(c, x1, x2, y, rr * 2, clef_line_width * 1.6f, col1);
			draw_shadow2(c, x1, x2, y, rr * 2, clef_line_width * 0.7f, col_shadow);
		}

		mp->draw_simple_note(c, x1, x2, y, 2, col1, col1, false);
		mp->draw_simple_note(c, x1, x2, y, -2, col, col_shadow, false);
	} else {
		// "shadow" to indicate length
		if (mp->allow_shadows and (x2 - x1 > mp->quality.shadow_threshold))
			draw_shadow(c, x1, x2, y, 0, rr, col_shadow);
			//draw_shadow2(c, x1, x2, y, rr * 2, clef_line_width, col_shadow);

		mp->draw_simple_note(c, x1, x2, y, 0, col, col_shadow, false);
	}

	mp->draw_note_flags(c, n, state, x1, x2, y);

	if ((n->modifier != NoteModifier::NONE) and mp->quality._highest_details) {
		c->set_color(local_theme.text);
		//c->setColor(ColorInterpolate(col, colors.text, 0.5f));
		SymbolRenderer::draw(c, {x - mp->modifier_font_size*1.0f, y - mp->modifier_font_size*0.5f}, mp->modifier_font_size, modifier_symbol(n->modifier));
	}
}


void MidiPainterModeClassical::draw_background(Painter *c, bool force) {
	// clef lines

	if (mp->is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);
	c->set_line_width(clef_line_width);
	c->set_antialiasing(true);

	for (int i=0; i<10; i+=2) {
		float y = clef_pos_to_screen(i);
		c->draw_line({mp->area.x1, y}, {mp->area.x2, y});
	}
	c->set_antialiasing(false);
	
	if (mp->is_playable)
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

void MidiPainterModeClassical::draw_key_symbol(Painter *c, const MidiKeyChange &kc) {
	float x = cam->sample2screen(kc.pos);

	c->set_font_size(clef_dy*4);
	c->draw_str({x + 10, clef_pos_to_screen(8)}, mp->clef->symbol);


	//SymbolRenderer::draw(c, x - size*1.0f, y - size*0.5f , size, modifier_symbol(n->modifier));
	//c->set_font_size(clef_dy);

	for (int i=0; i<7; i++) {
		if (kc.key.modifiers[i] != NoteModifier::NONE)
			SymbolRenderer::draw(c, {x + 18 + mp->modifier_font_size*3.0f + mp->modifier_font_size*0.6f*(i % 3), clef_pos_to_screen((i - mp->clef->offset + 7*20) % 7) - mp->modifier_font_size/2}, mp->modifier_font_size, modifier_symbol(kc.key.modifiers[i]));
	}
	c->set_font_size(local_theme.FONT_SIZE);
}


float MidiPainterModeClassical::clef_pos_to_screen(int pos) const {
	return mp->area.center().y - (pos - 4) * clef_dy / 2.0f;
}

int MidiPainterModeClassical::screen_to_clef_pos(float y) const {
	return (int)floor((mp->area.center().y - y) * 2.0f / clef_dy + 0.5f) + 4;
}

float MidiPainterModeClassical::pitch2y(int pitch) const {
	NoteModifier mod;
	int p = mp->clef->pitch_to_position(pitch, mp->midi_scale, mod);
	return clef_pos_to_screen(p);
}

int MidiPainterModeClassical::y2pitch(float y, NoteModifier modifier) const {
	int pos = screen_to_clef_pos(y);
	return mp->clef->position_to_pitch(pos, mp->midi_scale, modifier);
}

int MidiPainterModeClassical::y2clef(float y, NoteModifier &mod) const {
	mod = NoteModifier::UNKNOWN;//modifier;
	return screen_to_clef_pos(y);
}

void MidiPainterModeClassical::set_key_changes(const Array<MidiKeyChange> &changes) {
	key_changes = changes;
}
