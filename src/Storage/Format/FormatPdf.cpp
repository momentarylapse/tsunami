/*
 * FormatPdf.cpp
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#include "FormatPdf.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../lib/xfile/pdf.h"

FormatDescriptorPdf::FormatDescriptorPdf() :
	FormatDescriptor(_("Pdf sheet"), "pdf", Flag::MIDI | Flag::MULTITRACK | Flag::WRITE)
{
}

void FormatPdf::saveSong(StorageOperationData* od)
{
	auto parser = pdf::save(od->filename);
	auto p = parser->add_page(1200, 2000);

	float scale = 0.002f;
	float samples_per_line = p->width / scale;

	int num_lines = od->song->getRangeWithTime().end() / samples_per_line + 1;
	float y0 = 140;
	float string_dy = 16;
	float track_space = 20;
	float line_space = 20;

	float line_dy = 0;
	for (Track* t : od->song->tracks)
		line_dy += string_dy * t->instrument.string_pitch.num;


	p->setFontSize(40);
	p->drawStr(100, 30, od->song->getTag("title"));
	p->setFontSize(15);
	p->drawStr(p->width - 300, 70, "by " + od->song->getTag("artist"));

	for (int l=0; l<num_lines; l++){
		Range r = Range(l*samples_per_line, samples_per_line);

		auto beats = od->song->bars.getBeats(r, true, false);

		for (auto b: beats){
			if (b.level == 0){
				p->setColor(color(1, 0.5f, 0.5f, 0.5f));
				p->setLineWidth(2);
			}else{
				p->setColor(color(1, 0.8f, 0.8f, 0.8f));
				p->setLineWidth(1);
			}
			float x = (b.range.offset - r.offset) * scale;
			p->drawLine(x, y0, x, y0 + line_dy);
		}
		p->setLineWidth(1);

	for (Track* t : od->song->tracks){
		if (t->type != t->Type::MIDI)
			continue;

		p->setColor(Gray);
		for (int i=0; i<t->instrument.string_pitch.num; i++)
			p->drawLine(0, y0 + i*string_dy + string_dy/2, p->width, y0 + i*string_dy + string_dy/2);

		p->setFontSize(12);
		auto midi = t->midi.getNotes(r);
		for (auto n: midi){
			float x1 = (n->range.offset - r.offset) * scale;
			float x2 = (n->range.end() - r.offset) * scale;
			float y = y0 + (t->instrument.string_pitch.num - n->stringno - 1) * string_dy;
			p->setColor(color(1, 0.9f, 0.9f, 0.9f));
			p->drawRect(rect(x1, x2, y, y + string_dy));
			p->setColor(Black);
			p->drawStr(x1, y+2, i2s(n->pitch - t->instrument.string_pitch[n->stringno]));
		}
		y0 += string_dy * t->instrument.string_pitch.num + track_space;
		p->setColor(Black);
		//p->drawLine(0, y0, p->width, y0);
	}
	y0 += line_space;
	p->setColor(Black);
	//p->drawLine(0, y0, p->width, y0);


	}

	delete p;
	delete parser;
}
