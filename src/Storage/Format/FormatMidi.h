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

	void loadTrack(StorageOperationData *od);
	void saveBuffer(StorageOperationData *od);

	void loadAudio(StorageOperationData *od);
	void saveAudio(StorageOperationData *od);
};

#endif /* FORMATMIDI_H_ */
