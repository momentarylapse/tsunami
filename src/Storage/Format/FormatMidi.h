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
	virtual void loadTrack(StorageOperationData *od){}
	virtual void saveViaRenderer(StorageOperationData *od){}

	virtual void loadSong(StorageOperationData *od);
	virtual void saveSong(StorageOperationData *od);
};

class FormatDescriptorMidi : public FormatDescriptor
{
public:
	FormatDescriptorMidi();
	virtual Format *create(){ return new FormatMidi; }
};

#endif /* FORMATMIDI_H_ */
