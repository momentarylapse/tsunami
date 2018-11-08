/*
 * FormatRaw.h
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#ifndef FORMATRAW_H_
#define FORMATRAW_H_

#include "Format.h"

class FormatRaw: public Format
{
public:
	virtual void load_track(StorageOperationData *od);
	virtual void save_via_renderer(StorageOperationData *od);
};

class FormatDescriptorRaw : public FormatDescriptor
{
public:
	FormatDescriptorRaw();
	virtual Format *create(){ return new FormatRaw; }
};

#endif /* FORMATRAW_H_ */
