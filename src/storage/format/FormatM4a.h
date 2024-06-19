/*
 * FormatM4a.h
 *
 *  Created on: 21.09.2014
 *      Author: michi
 */

#ifndef FORMATM4A_H_
#define FORMATM4A_H_

#include "Format.h"

namespace tsunami {

class FormatM4a: public Format {
public:
	void load_track(StorageOperationData *od) override;
	void save_via_renderer(StorageOperationData *od) override {}
};

class FormatDescriptorM4a : public FormatDescriptor {
public:
	FormatDescriptorM4a();
	Format *create() override { return new FormatM4a; }
};

}

#endif /* FORMATMP3_H_ */
