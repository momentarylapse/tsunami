/*
 * FormatPdf.h
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATPDF_H_
#define SRC_STORAGE_FORMAT_FORMATPDF_H_

#include "Format.h"

class MidiPainter;
class ViewPort;
class Painter;
class Range;
class ColorScheme;

class FormatPdf : public Format {
public:
	void load_track(StorageOperationData *od) override {}
	void save_via_renderer(StorageOperationData *od) override {}

	void load_song(StorageOperationData *od) override {}
	void save_song(StorageOperationData* od) override;
	
	bool get_parameters(StorageOperationData *od, bool save) override;

	MidiPainter *mp;
	ViewPort *cam;
	ColorScheme *colors;
	StorageOperationData *od;
	
	Song *song;

	double pdf_bpm;
	Array<int> pdf_pattern;

	struct LineData {
		LineData(){}
		LineData(Track *t, float y0, float y1);
		float y0, y1;
		Track *track;
	};
	Array<LineData> line_data;

	int good_samples(const Range &r0);
	int draw_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	int draw_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	int draw_line(Painter *p, float x0, float w, float y0, const Range &r, float scale);

	void draw_beats(Painter *p, float x0, float w, float y, float h, const Range &r);
	void draw_bar_markers(Painter *p, float x0, float w, float y, float h, const Range &r);
};

class FormatDescriptorPdf : public FormatDescriptor {
public:
	FormatDescriptorPdf();
	Format *create() override { return new FormatPdf; }
};

#endif /* SRC_STORAGE_FORMAT_FORMATPDF_H_ */
