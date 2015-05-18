/*
 * FormatWave.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMATWAVE_H_
#define FORMATWAVE_H_

#include "Format.h"

class FormatWave: public Format
{
public:
	FormatWave();
	virtual ~FormatWave();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATWAVE_H_ */
