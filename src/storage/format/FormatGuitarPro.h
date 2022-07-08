/*
 * FormatGuitarPro.h
 *
 *  Created on: 01.10.2014
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMATGUITARPRO_H_
#define SRC_STORAGE_FORMATGUITARPRO_H_


#include "Format.h"

class Bar;

class FormatGuitarPro: public Format {
public:
	void load_track(StorageOperationData *od) override {}
	void save_via_renderer(StorageOperationData *od) override {}

	void load_song(StorageOperationData *od) override;
	void save_song(StorageOperationData *od) override;

	int version;

	struct GpTrack {
		Array<int> tuning;
		int midi_instrument;
		bool is_drum;
		Track *t;
	};
	Array<GpTrack> tracks;
	struct GpMeasure {
		int numerator, denominator;
		string marker;
		int length;
	};
	Array<GpMeasure> measures;
	int tempo;

	struct GpChannel {
		int instrument;
		int volume;
		int balance;
	};
	GpChannel channels[64];

	BinaryFormatter *f;
	StorageOperationData *od;
	Song *song;

	void read_info();
	void read_lyrics();
	void read_channels();
	void read_eq();
	void read_page_setup();
	void read_measure_header();
	void read_track();
	void read_measure(GpMeasure &m, GpTrack &t, int offset);
	int read_beat(GpTrack &t, GpMeasure &m, int start);
	void read_beat_fx();
	void read_note(GpTrack &t, int string_base, int start, int length);
	void read_note_fx();
	int read_duration(int flags, GpMeasure &m);
	void read_chord();
	void read_mix_change();

	void write_info();
	void write_lyrics();
	void write_channels();
	void write_eq();
	void write_page_setup();
	void write_measure_header(Bar *b);
	void write_track(GpTrack *t, int index);
	void write_measure(GpTrack *t, Bar *b);
	void write_beat(GpTrack *t, Array<int> &pitch, Array<int> &string, int length, bool update_tempo);
	void write_beat_fx();
	void write_note(GpTrack &t, int string_base, int start, int length);
	void write_note_fx();
	int write_duration(int flags, GpMeasure &m);
	void write_chord();
	void write_mix_change_tempo();
};


class FormatDescriptorGuitarPro : public FormatDescriptor {
public:
	FormatDescriptorGuitarPro();
	Format *create() override { return new FormatGuitarPro; }
};

#endif /* SRC_STORAGE_FORMATGUITARPRO_H_ */
