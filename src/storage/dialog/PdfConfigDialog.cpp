/*
 * PdfConfigDialog.cpp
 *
 *  Created on: 25.08.2018
 *      Author: michi
 */

#include "PdfConfigDialog.h"
#include "../StorageOperationData.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../lib/base/iter.h"
#include "../../lib/math/rect.h"
#include "../../view/painter/MultiLinePainter.h"
#include "../../view/helper/SymbolRenderer.h"
#include "../../view/ColorScheme.h"

ColorScheme create_pdf_color_scheme();
MultiLinePainter *prepare_pdf_multi_line_view(Song *song, const ColorScheme &_colors, const Any &params);
float draw_pdf_header(Painter *p, Song *song, float page_width, const ColorScheme &_colors);

PdfConfigDialog::PdfConfigDialog(StorageOperationData *_od, hui::Window *parent) :
	hui::Dialog("pdf export config", 200, 100, parent, false)
{
	od = _od;
	song = od->song;
	ok = false;

	from_resource("pdf-export-config-dialog");

	set_target("tracks");
	for (auto&& [i,t]: enumerate(weak(song->tracks))){
		if (t->type != SignalType::MIDI)
			continue;
		string id_classic = format("classical-%d", i);
		string id_tab = format("tab-%d", i);
		add_label(t->nice_name(), 0, i, "");
		add_check_box("Classical", 1, i, id_classic);
		add_check_box("TAB", 2, i, id_tab);
		check(id_classic, true);
		check(id_tab, t->instrument.string_pitch.allocated > 0);
		enable(id_tab, t->instrument.string_pitch.allocated > 0);
		// TODO read from parameters

		event(id_classic, [this] { update_params(); });
		event(id_tab, [this] { update_params(); });
	}
	set_float("scale", od->parameters["horizontal-scale"]._float() * 100);

	update_params();

	event_xp("area", "hui:draw", [this] (Painter *p) { on_draw(p); });
	event("scale", [this] { update_params(); });
	event("hui:close", [this] { on_close(); });
	event("cancel", [this] { on_close(); });
	event("ok", [this] { on_ok(); });
}

void PdfConfigDialog::on_close() {
	request_destroy();
}

void PdfConfigDialog::on_ok() {
	update_params();
	ok = true;

	request_destroy();
}

void PdfConfigDialog::update_params() {
	Any ats;
	for (auto&& [i,t]: enumerate(weak(song->tracks))){
		if (t->type != SignalType::MIDI)
			continue;
		Any at;
		at.map_set("index", i);
		if (is_checked(format("classical-%d", i)))
			at.map_set("classical", true);
		if (is_checked(format("tab-%d", i)))
			at.map_set("tab", true);
		ats.add(at);
	}
	od->parameters.map_set("tracks", ats);
	od->parameters.map_set("horizontal-scale", get_float("scale") / 100);

	redraw("area");
}

void PdfConfigDialog::on_draw(Painter *p) {

	float page_width = 595.276f;
	float page_height = 841.89f;

	auto _colors = create_pdf_color_scheme();
	auto mlp = prepare_pdf_multi_line_view(song, _colors, od->parameters);


	p->set_color(_colors.background);
	//p->draw_rect(p->area());
	p->draw_rect(rect(0, p->width, 0, p->width / page_width * page_height));

	float scale[4] = {p->width / page_width, 0, 0, p->width / page_width};
	p->set_transform(scale, {0,0});

	int samples = song->range().end();

	float y0 = draw_pdf_header(p, song, page_width, _colors);
	p->set_font("Helvetica", 8, false, false);

	int offset = 0;
	while (offset < samples) {
		float y_prev = y0;
		y0 = mlp->draw_next_line(p, offset, {0, y0});

		// new page?
		float dy = y0 - y_prev;
		if (y0 + dy > page_height and offset < samples) {
			break;
			/*p = parser.add_page();
			y0 = 50;*/
		}
	}

}

