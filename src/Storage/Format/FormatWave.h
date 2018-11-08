/*
 * FormatWave.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMATWAVE_H_
#define FORMATWAVE_H_

#include "Format.h"

class FormatWave: public Format
{
public:
	virtual void load_track(StorageOperationData *od);
	virtual void save_via_renderer(StorageOperationData *od);
};

class FormatDescriptorWave : public FormatDescriptor
{
public:
	FormatDescriptorWave();
	virtual Format *create(){ return new FormatWave; }
};

#endif /* FORMATWAVE_H_ */
