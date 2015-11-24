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

	virtual void loadTrack(StorageOperationData *od){}
	virtual void saveViaRenderer(StorageOperationData *od){}

	virtual void loadSong(StorageOperationData *od);
	virtual void saveSong(StorageOperationData *od);
};

#endif /* FORMATMIDI_H_ */
