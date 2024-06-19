/*
 * FormatFlac.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATFLAC_H_
#define FORMATFLAC_H_

#include "Format.h"

namespace tsunami {

class FormatFlac: public Format {
public:
	void load_track(StorageOperationData *od) override;
	void save_via_renderer(StorageOperationData *od) override;
};

class FormatDescriptorFlac : public FormatDescriptor {
public:
	FormatDescriptorFlac();
	Format *create() override { return new FormatFlac; }
};

}

#endif /* FORMATFLAC_H_ */
