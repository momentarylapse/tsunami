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

MidiPainterModeClassical::MidiPainterModeClassical(MidiPainter *mp, Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors) :
    MidiPainterMode(mp, song, cam, sel, hover, colors)
{
}

void MidiPainterModeClassical::draw(Painter *c, const MidiNoteBuffer &midi) {

}

void MidiPainterModeClassical::draw_background(Painter *c, bool force) {
	// clef lines

	if (mp->is_playable)
		c->set_color(local_theme.text_soft1);
	else
		c->set_color(local_theme.text_soft3);
	c->set_line_width(mp->clef_line_width);
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
	for (auto &kc: mp->key_changes)
		if (kc.pos < cam->range().offset)
			key_prev = kc.key;
	draw_key_symbol(c, MidiKeyChange(cam->pos, key_prev));

	for (auto &kc: mp->key_changes)
		draw_key_symbol(c, kc);

	c->set_font_size(local_theme.FONT_SIZE);
}

void MidiPainterModeClassical::draw_key_symbol(Painter *c, const MidiKeyChange &kc) {
	float x = cam->sample2screen(kc.pos);

	c->set_font_size(mp->clef_dy*4);
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
	return mp->area.center().y - (pos - 4) * mp->clef_dy / 2.0f;
}

int MidiPainterModeClassical::screen_to_clef_pos(float y) const {
	return (int)floor((mp->area.center().y - y) * 2.0f / mp->clef_dy + 0.5f) + 4;
}
