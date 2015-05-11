/*
 * FormatGp4.h
 *
 *  Created on: 01.10.2014
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMATGP4_H_
#define SRC_STORAGE_FORMATGP4_H_


#include "Format.h"

class FormatGp4: public Format
{
public:
	FormatGp4();
	virtual ~FormatGp4();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);

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

	AudioFile *a;
	CFile *f;

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

#endif /* SRC_STORAGE_FORMATGP4_H_ */
