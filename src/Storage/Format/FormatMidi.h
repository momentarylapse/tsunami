/*
 * FormatMidi.h
 *
 *  Created on: 17.02.2013
 *      Author: michi
 */

#ifndef FORMATMIDI_H_
#define FORMATMIDI_H_

#include "Format.h"

class FormatMidi: public Format {
public:
	void load_track(StorageOperationData *od) override {}
	void save_via_renderer(StorageOperationData *od) override {}

	void load_song(StorageOperationData *od) override;
	void save_song(StorageOperationData* od) override;
};

class FormatDescriptorMidi : public FormatDescriptor {
public:
	FormatDescriptorMidi();
	Format *create() override { return new FormatMidi; }
};

#endif /* FORMATMIDI_H_ */
