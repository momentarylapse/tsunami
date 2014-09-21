/*
 * FormatM4a.h
 *
 *  Created on: 21.09.2014
 *      Author: michi
 */

#ifndef FORMATM4A_H_
#define FORMATM4A_H_

#include "Format.h"

class FormatM4a: public Format
{
public:
	FormatM4a();
	virtual ~FormatM4a();

	void LoadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATMP3_H_ */
