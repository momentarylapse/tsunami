#include "MidiPainterMode.h"
#include "../MidiPainter.h"
#include "../../HoverData.h"
#include "../../ColorScheme.h"
#include "../../../data/SongSelection.h"
#include "../../../data/midi/MidiNote.h"
#include "../../../lib/image/Painter.h"
#include "../../../lib/math/vec2.h"

namespace tsunami {

void get_col(color &col, color &col_shadow, const MidiNote *n, MidiNoteState state, bool playable, const ColorScheme &colors) {
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
	if ((hover->type == HoverData::Type::MidiNote) and (n == hover->note))
		return MidiNoteState::HOVER | s;
	return s;
}

// "shadow" to indicate length
void draw_shadow(Painter *c, float x1, float x2, float y, float shadow_width, const color &col) {
	//x1 += r;
	c->set_color(col);
	c->draw_rect(rect(x1, x2, y - shadow_width/2, y + shadow_width/2));
}

void MidiPainterMode::draw_shadow2(Painter *c, float x1, float x2, float y, float hole, float scale, const color &col) {
	c->set_color(col);
	c->set_line_width(shadow_width * scale);
	float x = (x1 + x2) / 2;
	c->draw_line({x1 + shadow_offset, y}, {x - hole/2, y});
	c->draw_line({x + hole/2, y}, {x2 - shadow_offset, y});
}

MidiPainterMode::MidiPainterMode(MidiPainter *mp) :
    mp(mp), cam(mp->cam), song(mp->song), sel(mp->sel), hover(mp->hover), local_theme(mp->local_theme)
{}

}
