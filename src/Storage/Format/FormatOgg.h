/*
 * FormatOgg.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATOGG_H_
#define FORMATOGG_H_

#include "Format.h"

class FormatOgg: public Format
{
public:
	FormatOgg();
	virtual ~FormatOgg();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATOGG_H_ */
