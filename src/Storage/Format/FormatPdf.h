/*
 * FormatPdf.h
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATPDF_H_
#define SRC_STORAGE_FORMAT_FORMATPDF_H_

#include "Format.h"

class FormatPdf : public Format
{
public:
	virtual void load_track(StorageOperationData *od){}
	virtual void save_via_renderer(StorageOperationData *od){}

	virtual void load_song(StorageOperationData *od){}
	virtual void save_song(StorageOperationData* od);
};

class FormatDescriptorPdf : public FormatDescriptor
{
public:
	FormatDescriptorPdf();
	virtual Format *create(){ return new FormatPdf; }
};

#endif /* SRC_STORAGE_FORMAT_FORMATPDF_H_ */
