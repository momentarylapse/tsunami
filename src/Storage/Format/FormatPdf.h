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

class FormatPdf : public Format
{
public:
	virtual void load_track(StorageOperationData *od){}
	virtual void save_via_renderer(StorageOperationData *od){}

	virtual void load_song(StorageOperationData *od){}
	virtual void save_song(StorageOperationData* od);

	MidiPainter *mp;
	ViewPort *cam;

	double pdf_bpm;

	float clef_pos_to_pdf(float y0, float line_dy, int i);
	int good_samples(const Range &r0);
	int render_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	int render_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	int render_line(Painter *p, float x0, float w, float y0, const Range &r, float scale, PdfConfigData *data);
};

class FormatDescriptorPdf : public FormatDescriptor
{
public:
	FormatDescriptorPdf();
	virtual Format *create(){ return new FormatPdf; }
};

#endif /* SRC_STORAGE_FORMAT_FORMATPDF_H_ */
