/*
 * FormatSoundFont2.h
 *
 *  Created on: 19.05.2015
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_
#define SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_

#include "Format.h"

namespace tsunami {

class FormatSoundFont2: public Format {
public:
	FormatSoundFont2();

	void load_track(StorageOperationData *od) override {}
	void save_via_renderer(StorageOperationData *od) override {}

	void load_song(StorageOperationData *od) override;
	void save_song(StorageOperationData *od) override {}

	void read_chunk(Stream *f);
	void read_samples(Stream *f);

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
	struct sfPresetHeader {
		string name;
		int preset;
		int bank;
		int zone_start;
		int zone_end;
		int library;
		int genre;
		int morphology;
	};
	Array<sfPresetHeader> presets;
	struct sfZone {
		int gen_start, gen_end, mod_index;
	};
	Array<sfZone> preset_zones, instrument_zones;
	struct sfInstrument {
		string name;
		int zone_start;
		int zone_end;
	};
	Array<sfInstrument> instruments;
	struct sfGenerator {
		int op, amount;
		string str() const;
	};
	Array<sfGenerator> preset_generators, instrument_generators;

	void read_sample_header(Stream *f, sfSample &s);
};

class FormatDescriptorSoundFont2 : public FormatDescriptor {
public:
	FormatDescriptorSoundFont2();
	Format *create() override { return new FormatSoundFont2; }
};

}

#endif /* SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_ */
