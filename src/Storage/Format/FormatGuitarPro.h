/*
 * FormatGuitarPro.h
 *
 *  Created on: 01.10.2014
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMATGUITARPRO_H_
#define SRC_STORAGE_FORMATGUITARPRO_H_


#include "Format.h"

class FormatGuitarPro: public Format
{
public:
	FormatGuitarPro();
	virtual ~FormatGuitarPro();

	void loadTrack(StorageOperationData *od);
	void saveBuffer(StorageOperationData *od);

	void loadSong(StorageOperationData *od);
	void saveSong(StorageOperationData *od);

	int version;

	struct GpTrack
	{
		int stringCount;
		Array<int> tuning;
		Track *t;
	};
	Array<GpTrack> tracks;
	struct GpMeasure
	{
		int numerator, denominator;
		string marker;
		int length;
	};
	Array<GpMeasure> measures;
	int tempo;

	Song *a;
	File *f;
	StorageOperationData *od;

	void read_info();
	void read_lyrics();
	void read_channels();
	void read_eq();
	void read_page_setup();
	void read_measure_header();
	void read_track();
	void read_channel();
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
	void write_measure_header(Bar &b);
	void write_track(Track *t);
	void write_channel();
	void write_measure(Track *t, Bar &b);
	void write_beat(Array<int> &pitch, Array<int> &string, int length);
	void write_beat_fx();
	void write_note(GpTrack &t, int string_base, int start, int length);
	void write_note_fx();
	int write_duration(int flags, GpMeasure &m);
	void write_chord();
	void write_mix_change();
};

#endif /* SRC_STORAGE_FORMATGUITARPRO_H_ */
