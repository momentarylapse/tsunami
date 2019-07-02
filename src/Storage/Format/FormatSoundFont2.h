/*
 * FormatSoundFont2.h
 *
 *  Created on: 19.05.2015
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_
#define SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_

#include "Format.h"

class FormatSoundFont2: public Format {
public:
	FormatSoundFont2();

	void load_track(StorageOperationData *od) override {}
	void save_via_renderer(StorageOperationData *od) override {}

	void load_song(StorageOperationData *od) override;
	void save_song(StorageOperationData *od) override {}

	void read_chunk(File *f);
	void read_samples(File *f);

	int sample_offset;
	int sample_count;
	StorageOperationData *od;
	Song *song;


	struct sfSample {
		string name;
		int start;
		int end;
		int start_loop;
		int end_loop;
		int sample_rate;
		int original_key;
		int correction;
		int sample_link;
		int sample_type;
		void print();
	};
	Array<sfSample> samples;

	void read_sample_header(File *f, sfSample &s);
};

class FormatDescriptorSoundFont2 : public FormatDescriptor {
public:
	FormatDescriptorSoundFont2();
	Format *create() override { return new FormatSoundFont2; }
};

#endif /* SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_ */
