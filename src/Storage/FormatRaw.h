/*
 * FormatRaw.h
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#ifndef FORMATRAW_H_
#define FORMATRAW_H_

#include "Format.h"

class FormatRaw: public Format
{
public:
	FormatRaw();
	virtual ~FormatRaw();

	void LoadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATRAW_H_ */
