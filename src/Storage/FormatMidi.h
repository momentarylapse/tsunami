/*
 * FormatMidi.h
 *
 *  Created on: 17.02.2013
 *      Author: michi
 */

#ifndef FORMATMIDI_H_
#define FORMATMIDI_H_

#include "Format.h"

class FormatMidi: public Format
{
public:
	FormatMidi();
	virtual ~FormatMidi();

	void loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	void saveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void loadAudio(AudioFile *a, const string &filename);
	void saveAudio(AudioFile *a, const string &filename);
};

#endif /* FORMATMIDI_H_ */
