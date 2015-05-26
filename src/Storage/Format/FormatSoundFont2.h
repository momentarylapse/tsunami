/*
 * FormatSoundFont2.h
 *
 *  Created on: 19.05.2015
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_
#define SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_

#include "Format.h"

class FormatSoundFont2: public Format
{
public:
	FormatSoundFont2();
	virtual ~FormatSoundFont2();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);

	void read_chunk(File *f);
	void read_samples(File *f);

	int sample_offset;
	int sample_count;


	struct sfSample
	{
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

	AudioFile *audio;
};

#endif /* SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_ */
