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

	void read_chunk(CFile *f);
	void read_samples(CFile *f);

	int sample_offset;


	struct sfSample
	{
		char name[20];
		int start;
		int end;
		int start_loop;
		int end_loop;
		int sample_rate;
		unsigned char by_original_key;
		char correction;
		short sample_link;
		short sample_type;
		void print();
	};
	Array<sfSample> samples;

	AudioFile *audio;
};

#endif /* SRC_STORAGE_FORMAT_FORMATSOUNDFONT2_H_ */
