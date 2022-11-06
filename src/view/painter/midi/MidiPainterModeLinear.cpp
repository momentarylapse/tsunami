#include "MidiPainterModeLinear.h"
#include "../MidiPainter.h"
#include "../../ColorScheme.h"
#include "../../HoverData.h"
#include "../../audioview/ViewPort.h"
#include "../../helper/SymbolRenderer.h"
#include "../../../data/midi/MidiData.h"
#include "../../../data/midi/Instrument.h"
#include "../../../data/midi/Clef.h"
#include "../../../data/SampleRef.h"
#include "../../../data/Sample.h"
#include "../../../module/synthesizer/Synthesizer.h"
#include "../../../module/ModuleConfiguration.h"
#include "../../../lib/image/Painter.h"
#include "../../../lib/math/vec2.h"


MidiPainterModeLinear::MidiPainterModeLinear(MidiPainter *mp) :
    MidiPainterMode(mp)
{
}

void MidiPainterModeLinear::reset() {
	pitch_min = PITCH_MIN_DEFAULT;
	pitch_max = PITCH_MAX_DEFAULT;
}

void MidiPainterModeLinear::update() {
	clef_line_width = mp->area.height() / 150;

	// absolute pixel value (3.5) allowed, since we only use this on screen :P
	rr = max((pitch2y(0) - pitch2y(1)) / 1.0f, 3.5f);
}

void MidiPainterModeLinear::draw_notes(Painter *c, const MidiNoteBuffer &midi) {
	if (mp->quality._highest_details)
		mp->draw_rhythm(c, midi, mp->cur_range, [this] (MidiNote *n) {
			return pitch2y(n->pitch);
		});
	//c->setLineWidth(3.0f);

	// draw notes
	c->set_antialiasing(mp->quality.antialiasing);
	for (auto *n: weak(midi)) {
		if ((n->pitch < pitch_min) or (n->pitch >= pitch_max))
			continue;
		draw_note(c, *n, note_state(n, mp->as_reference, sel, hover));
	}
	c->set_antialiasing(false);
}


void MidiPainterModeLinear::draw_note(Painter *c, const MidiNote &n, MidiNoteState state) {
	float x1, x2;
	cam->range2screen(n.range, x1, x2);
	float y = pitch2y(n.pitch);
	n.y = y;

    float rr = mp->rr;

	color col, col_shadow;
	get_col(col, col_shadow, &n, state, mp->is_playable, local_theme);

	if (state & MidiNoteState::SELECTED) {
		color col1 = local_theme.pitch_text[(int)n.pitch % 12];//selection;

		// "shadow" to indicate length
		if (mp->allow_shadows and (x2 - x1 > mp->quality.shadow_threshold)) {
			//draw_shadow(c, x1, x2, y, rx, rr, col_shadow);
			draw_shadow2(c, x1, x2, y, rr * 2, clef_line_width * 3 * 1.6f, col1);
			draw_shadow2(c, x1, x2, y, rr * 2, clef_line_width * 3 * 0.7f, col_shadow);
		}

		mp->draw_simple_note(c, x1, x2, y, 2, col1, col1, false);
		mp->draw_simple_note(c, x1, x2, y, -2, col, col_shadow, false);
	} else {
		// "shadow" to indicate length
		if (mp->allow_shadows and (x2 - x1 > mp->quality.shadow_threshold))
			draw_shadow(c, x1, x2, y, 0, rr, col_shadow);
			//draw_shadow2(c, x1, x2, y, rr * 2, clef_line_width * 3, col_shadow);

		mp->draw_simple_note(c, x1, x2, y, 0, col, col_shadow, false);
	}

	mp->draw_note_flags(c, &n, state, x1, x2, y);
}

void MidiPainterModeLinear::draw_background(Painter *c, bool force) {
    if (force)
        draw_pitch_grid(c);
}

void MidiPainterModeLinear::draw_pitch_grid(Painter *c) {
	// pitch grid
	c->set_color(color(0.25f, 0, 0, 0));
	for (int i=pitch_min; i<pitch_max; i++) {
		float y0 = pitch2y(i + 0.5f);
		float y1 = pitch2y(i - 0.5f);
		if (!mp->midi_scale.contains(i)) {
			c->set_color(color(0.2f, 0, 0, 0));
			c->draw_rect(rect(mp->area.x1, mp->area.x2, y0, y1));
		}
	}


	// pitch names
	color cc = local_theme.text;
	cc.a = 0.4f;
	Array<SampleRef*> *p = nullptr;
	if (mp->synth and (mp->synth->module_class == "Sample")) {
		auto *c = mp->synth->get_config();
		p = (Array<SampleRef*> *)&c[1];
	}
	bool is_drum = (mp->instrument->type == Instrument::Type::DRUMS);
	float dy = ((pitch2y(0) - pitch2y(1)) - c->font_size) / 2;
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
		c->draw_str({20, pitch2y(i+0.5f)+dy}, name);
	}
}

// this is the "center" of the note!
// it reaches below and above this y
float MidiPainterModeLinear::pitch2y(float pitch) const {
	return mp->area.y2 - mp->area.height() * (pitch - (float)pitch_min) / (pitch_max - pitch_min);
}

int MidiPainterModeLinear::y2pitch(float y) const {
	return pitch_min + ((mp->area.y2 - y) * (pitch_max - pitch_min) / mp->area.height()) + 0.5f;
}

int MidiPainterModeLinear::y2clef(float y, NoteModifier &mod) const {
	mod = NoteModifier::UNKNOWN;//modifier;

	int pitch = y2pitch(y);
	return mp->clef->pitch_to_position(pitch, mp->midi_scale, mod);
}

void MidiPainterModeLinear::set_linear_range(float _pitch_min, float _pitch_max) {
    pitch_min = _pitch_min;
    pitch_max = _pitch_max;
}
