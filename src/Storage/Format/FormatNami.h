/*
 * FormatNami.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMATNAMI_H_
#define FORMATNAMI_H_

#include "Format.h"

class FormatNami : public Format
{
public:
	virtual void loadTrack(StorageOperationData *od){}
	virtual void saveViaRenderer(StorageOperationData *od){}

	virtual void loadSong(StorageOperationData *od);
	virtual void saveSong(StorageOperationData *od);

	void make_consistent(Song *s);
};

class FormatDescriptorNami : public FormatDescriptor
{
public:
	FormatDescriptorNami();
	virtual Format *create(){ return new FormatNami; }
};

#endif /* FORMATNAMI_H_ */
