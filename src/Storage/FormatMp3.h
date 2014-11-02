/*
 * FormatMp3.h
 *
 *  Created on: 18.09.2014
 *      Author: michi
 */

#ifndef FORMATMP3_H_
#define FORMATMP3_H_

#include "Format.h"

class FormatMp3: public Format
{
public:
	FormatMp3();
	virtual ~FormatMp3();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATMP3_H_ */
