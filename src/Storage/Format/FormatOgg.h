/*
 * FormatOgg.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATOGG_H_
#define FORMATOGG_H_

#include "Format.h"

class FormatOgg: public Format
{
public:
	virtual void loadTrack(StorageOperationData *od);
	virtual void saveViaRenderer(StorageOperationData *od);
};

class FormatDescriptorOgg : public FormatDescriptor
{
public:
	FormatDescriptorOgg();
	virtual Format *create(){ return new FormatOgg; }
};

#endif /* FORMATOGG_H_ */
