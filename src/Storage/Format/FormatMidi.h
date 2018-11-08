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
	virtual void load_track(StorageOperationData *od){}
	virtual void save_via_renderer(StorageOperationData *od){}

	virtual void load_song(StorageOperationData *od);
	virtual void save_song(StorageOperationData* od);
};

class FormatDescriptorMidi : public FormatDescriptor
{
public:
	FormatDescriptorMidi();
	virtual Format *create(){ return new FormatMidi; }
};

#endif /* FORMATMIDI_H_ */
