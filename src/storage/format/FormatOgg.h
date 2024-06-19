/*
 * FormatOgg.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATOGG_H_
#define FORMATOGG_H_

#include "Format.h"

namespace tsunami {

class FormatOgg: public Format {
public:
	void load_track(StorageOperationData *od) override;
	void save_via_renderer(StorageOperationData *od) override;
};

class FormatDescriptorOgg : public FormatDescriptor {
public:
	FormatDescriptorOgg();
	Format *create() override { return new FormatOgg; }
};

}

#endif /* FORMATOGG_H_ */
