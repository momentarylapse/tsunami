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
class PdfConfigData;
class ColorScheme;

class FormatPdf : public Format
{
public:
	virtual void load_track(StorageOperationData *od){}
	virtual void save_via_renderer(StorageOperationData *od){}

	virtual void load_song(StorageOperationData *od){}
	virtual void save_song(StorageOperationData* od);

	MidiPainter *mp;
	ViewPort *cam;
	ColorScheme *colors;

	double pdf_bpm;

	struct LineData
	{
		LineData(){}
		LineData(Track *t, float y0, float y1);
		float y0, y1;
		Track *track;
	};
	Array<LineData> line_data;

	int good_samples(const Range &r0);
	int draw_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	int draw_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	int draw_line(Painter *p, float x0, float w, float y0, const Range &r, float scale, PdfConfigData *data);

	void draw_beats(Painter *p, float x0, float w, float y, float h, const Range &r);
};

class FormatDescriptorPdf : public FormatDescriptor
{
public:
	FormatDescriptorPdf();
	virtual Format *create(){ return new FormatPdf; }
};

#endif /* SRC_STORAGE_FORMAT_FORMATPDF_H_ */
