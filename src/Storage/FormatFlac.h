/*
 * FormatFlac.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATFLAC_H_
#define FORMATFLAC_H_

#include "Format.h"

class FormatFlac: public Format
{
public:
	FormatFlac();
	virtual ~FormatFlac();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATFLAC_H_ */
