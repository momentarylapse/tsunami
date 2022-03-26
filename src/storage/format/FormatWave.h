/*
 * FormatWave.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMATWAVE_H_
#define FORMATWAVE_H_

#include "Format.h"

class FormatWave: public Format {
public:
	void load_track(StorageOperationData *od) override;
	void save_via_renderer(StorageOperationData *od) override;
};

class FormatDescriptorWave : public FormatDescriptor {
public:
	FormatDescriptorWave();
	Format *create() override { return new FormatWave; }
};

#endif /* FORMATWAVE_H_ */
