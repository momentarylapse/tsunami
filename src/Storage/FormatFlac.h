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

	void LoadTrack(Track *t, const string &filename);
	void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATFLAC_H_ */
