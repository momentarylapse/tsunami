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
};

#endif /* SRC_STORAGE_FORMATGP4_H_ */
