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
	virtual void load_track(StorageOperationData *od){}
	virtual void save_via_renderer(StorageOperationData *od){}

	virtual void load_song(StorageOperationData *od);
	virtual void save_song(StorageOperationData *od);

	void make_consistent(Song *s);
};

class FormatDescriptorNami : public FormatDescriptor
{
public:
	FormatDescriptorNami();
	virtual Format *create(){ return new FormatNami; }
};

#endif /* FORMATNAMI_H_ */
