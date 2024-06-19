/*
 * FormatPdf.h
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATPDF_H_
#define SRC_STORAGE_FORMAT_FORMATPDF_H_

#include "Format.h"

class Painter;

namespace tsunami {

class MidiPainter;
class ViewPort;
class Range;
class ColorScheme;

class FormatPdf : public Format {
public:
	void load_track(StorageOperationData *od) override {}
	void save_via_renderer(StorageOperationData *od) override {}

	void load_song(StorageOperationData *od) override {}
	void save_song(StorageOperationData* od) override;
	
	bool get_parameters(StorageOperationData *od, bool save) override;

	StorageOperationData *od;
	
	Song *song;
};

class FormatDescriptorPdf : public FormatDescriptor {
public:
	FormatDescriptorPdf();
	Format *create() override { return new FormatPdf; }
};

}

#endif /* SRC_STORAGE_FORMAT_FORMATPDF_H_ */
