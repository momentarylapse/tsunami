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
	hui::Dialog("pdf-export-config-dialog", parent)
{
	od = _od;
	song = od->song;
	ok = false;

	add_string("theme", "default");
	add_string("theme", "dark (preview only)");
	set_int("theme", 0);
	event("theme", [this] { update_params(); });

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
	set_float("line-height", od->parameters["line-height"]._float());
	set_float("line-space", od->parameters["line-space"]._float() / od->parameters["line-height"]._float());
	set_float("track-space", od->parameters["track-space"]._float() / od->parameters["line-height"]._float());
	set_float("border", od->parameters["border"]._float());
	set_float("horizontal-scale", od->parameters["horizontal-scale"]._float() * 100);
	check("allow-shadows", od->parameters["allow-shadows"]._bool());

	update_params();

	//event_xp("area", "hui:draw", [this] (Painter *p) { on_draw(p); });
	event_x("area", "hui:mouse-wheel", [this] () { on_mouse_wheel(); });
	event("line-height", [this] { update_params(); });
	event("line-space", [this] { update_params(); });
	event("track-space", [this] { update_params(); });
	event("track-height", [this] { update_params(); });
	event("horizontal-scale", [this] { update_params(); });
	event("allow-shadows", [this] { update_params(); });
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
	od->parameters.map_set("line-height", get_float("line-height"));
	od->parameters.map_set("line-space", get_float("line-space") * get_float("line-height"));
	od->parameters.map_set("track-space", get_float("track-space") * get_float("line-height"));
	//od->parameters.map_set("border", get_float("border"));
	od->parameters.map_set("horizontal-scale", get_float("horizontal-scale") / 100);
	od->parameters.map_set("allow-shadows", is_checked("allow-shadows"));

	od->parameters.map_set("theme", get_int("theme"));

	redraw("area");
}

void PdfConfigDialog::on_mouse_wheel() {
	preview_offset_y = max(preview_offset_y + hui::get_event()->scroll.y, 0.0f);
	redraw("area");
}

void PdfConfigDialog::on_draw(Painter *p) {

	float page_width = 595.276f;
	float page_height = 841.89f;

	auto _colors = create_pdf_color_scheme();
	if (od->parameters["theme"]._int() == 1)
		_colors = ColorSchemeDark();
	auto mlp = prepare_pdf_multi_line_view(song, _colors, od->parameters);


	p->set_color(_colors.background);
	//p->draw_rect(p->area());
	float scale = p->width / page_width;
	//p->draw_rect(rect(0, p->width, 0, scale * page_height));

	float mat[4] = {scale, 0, 0, scale};
	p->set_transform(mat, {0, -preview_offset_y * scale});
	p->draw_rect(rect(0, page_width, 0, page_height));

	int samples = song->range().end();

	float y0 = draw_pdf_header(p, song, page_width, _colors);
	p->set_font("Helvetica", 8, false, false);

	int offset = 0;
	int page_no = 0;
	while (offset < samples) {
		float y_prev = y0;
		y0 = mlp->draw_next_line(p, offset, {0, y0});

		// new page?
		float dy = y0 - y_prev;
		if (y0 + dy > page_height and offset < samples) {
			//break;
			page_no ++;
			y0 = 50;
			p->set_transform(mat, {0, (page_no * (page_height + 16) - preview_offset_y) * scale});
			p->set_color(_colors.background);
			p->draw_rect(rect(0, page_width, 0, page_height));
		}
	}

	float m_id[4] = {1, 0, 0, 1};
	p->set_transform(mat, {0, 0});
}

