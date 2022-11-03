/*
 * FormatPdf.cpp
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#include "FormatPdf.h"
#include "../dialog/PdfConfigDialog.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/SongSelection.h"
#include "../../lib/doc/pdf.h"
#include "../../lib/math/rect.h"
#include "../../view/ColorScheme.h"
#include "../../view/helper/SymbolRenderer.h"
#include "../../view/HoverData.h"
#include "../../view/painter/MultiLinePainter.h"
#include <math.h>

static const color NOTE_COLOR = color(1, 0.3f, 0.3f, 0.3f);
static const color NOTE_COLOR_TAB = color(1, 0.8f, 0.8f, 0.8f);

FormatDescriptorPdf::FormatDescriptorPdf() :
	FormatDescriptor(_("Pdf sheet"), "pdf", Flag::MIDI | Flag::MULTITRACK | Flag::WRITE) {}


bool FormatPdf::get_parameters(StorageOperationData *od, bool save) {
	// optional defaults
	if (!od->parameters.has("horizontal-scale"))
		od->parameters.map_set("horizontal-scale", 1.0f);

	if (od->parameters.has("tracks"))
		return true;

	// mandatory defaults
	if (!od->parameters.has("tracks"))
		od->parameters.map_set("tracks", {});
	
	bool ok = false;
	auto dlg = new PdfConfigDialog(od, od->win);
	hui::run(dlg, [dlg, &ok] {
		ok = dlg->ok;
	});
	return ok;
}

ColorScheme create_pdf_color_scheme() {
	ColorScheme bright;
	bright.background = White;
	bright.text = Black;//color(1, 0.3f, 0.3f, 0.1f);
	bright.selection = color(1, 0.2f, 0.2f, 0.7f);
	bright.hover = White;
	bright.gamma = 1.0f;
	bright.name = "pdf";
	bright.auto_generate();
	return bright;
}
void FormatPdf::save_song(StorageOperationData* _od) {
	od = _od;
	song = od->song;

	float page_width = 1200;
	float page_height = 2000;
	// A4
	page_width = 595.276f;
	page_height = 841.89f;

	float border = 25;

	ColorScheme _colors = create_pdf_color_scheme();

	float avg_scale = 65.0f / od->song->sample_rate * od->parameters["horizontal-scale"]._float();
	float avg_samples_per_line = (page_width - 2*border) / avg_scale;

	MultiLinePainter mlp(song, _colors);
	mlp.set_context(od->parameters["tracks"], page_width, avg_samples_per_line);
	Any conf;
	conf["border"] = border;
	conf["line-height"] = 20;
	mlp.set(conf);

	pdf::Parser parser;
	parser.set_page_size(page_width, page_height);


	SymbolRenderer::enable(false);


	int samples = od->song->range_with_time().end();
	//int num_lines =  / samples_per_line + 1;
	float y0 = 70;
	mlp.line_space = 25;

	bool first_page = true;


	auto p = parser.add_page();

	if (first_page) {
		p->set_color(_colors.text);
		p->set_font("Times", 26, false, false);
		//p->set_font("Helvetica", 25, false, false);
		p->draw_str(vec2(100, 25), od->song->get_tag("title"));
		if (od->song->get_tag("artist").num > 0) {
			p->set_font("Courier", 15, false, false);
			p->set_font_size(15);
			p->set_color(_colors.text_soft2);
			p->draw_str(vec2(p->width - 150, 25), "by " + od->song->get_tag("artist"));
		}
		first_page = false;
	}
	p->set_font("Helvetica", 8, false, false);

	int offset = 0;
	while (offset < samples) {
		float y_prev = y0;
		y0 = mlp.draw_next_line(p, offset, {0, y0});

		// new page?
		float dy = y0 - y_prev;
		if (y0 + dy > page_height and offset < samples) {
			p = parser.add_page();
			y0 = 50;
		}
	}

	parser.save(od->filename);
}
