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

	void LoadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);

	void read_info(AudioFile *a, CFile *f);
	void read_lyrics(AudioFile *a, CFile *f);
	void read_channels(AudioFile *a, CFile *f);
	void read_measure_header(AudioFile *a, CFile *f);
	void read_track(AudioFile *a, CFile *f);
	void read_channel(AudioFile *a, CFile *f);
	void read_measure(AudioFile *a, CFile *f, int i, int j);
	void read_beat(AudioFile *a, Track *t, CFile *f);
	void read_beat_fx(AudioFile *a, CFile *f);
	void read_note(AudioFile *a, CFile *f, int string_base);
	void read_note_fx(AudioFile *a, CFile *f);
	void read_duration(AudioFile *a, CFile *f, int flags);
	void read_chord(AudioFile *a, CFile *f);
	void read_mix_change(AudioFile *a, CFile *f);

	struct GpTrack
	{
		int stringCount;
		Array<int> tuning;
	};
	Array<GpTrack> tracks;
	struct GpMeasure
	{
		int numerator, denominator;
		string marker;
	};
	Array<GpMeasure> measures;
	int tempo;
};

#endif /* SRC_STORAGE_FORMATGP4_H_ */
