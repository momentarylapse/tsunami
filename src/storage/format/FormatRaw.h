/*
 * FormatRaw.h
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#ifndef FORMATRAW_H_
#define FORMATRAW_H_

#include "Format.h"

namespace tsunami {

class FormatRaw: public Format {
public:
	bool get_parameters(StorageOperationData *od, bool save) override;
	void load_track(StorageOperationData *od) override;
	void save_via_renderer(StorageOperationData *od) override;
};

class FormatDescriptorRaw : public FormatDescriptor {
public:
	FormatDescriptorRaw();
	Format *create() override { return new FormatRaw; }
};

}

#endif /* FORMATRAW_H_ */
