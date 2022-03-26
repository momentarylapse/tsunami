/*
 * FormatNami.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMATNAMI_H_
#define FORMATNAMI_H_

#include "Format.h"

class FormatNami : public Format {
public:
	void load_track(StorageOperationData *od) override {}
	void save_via_renderer(StorageOperationData *od) override {}

	void load_song(StorageOperationData *od) override;
	void save_song(StorageOperationData *od) override;

	void make_consistent(StorageOperationData *od);
};

class FormatDescriptorNami : public FormatDescriptor {
public:
	FormatDescriptorNami();
	Format *create() override { return new FormatNami; }
};

#endif /* FORMATNAMI_H_ */
