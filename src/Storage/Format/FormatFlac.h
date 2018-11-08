/*
 * FormatFlac.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATFLAC_H_
#define FORMATFLAC_H_

#include "Format.h"

class FormatFlac: public Format
{
public:
	virtual void load_track(StorageOperationData *od);
	virtual void save_via_renderer(StorageOperationData *od);
};

class FormatDescriptorFlac : public FormatDescriptor
{
public:
	FormatDescriptorFlac();
	virtual Format *create(){ return new FormatFlac; }
};

#endif /* FORMATFLAC_H_ */
