/*
 * FormatNami.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMATNAMI_H_
#define FORMATNAMI_H_

#include "Format.h"

class FormatNami : public Format
{
public:
	FormatNami();
	virtual ~FormatNami();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATNAMI_H_ */
