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
	FormatNami();
	virtual ~FormatNami();

	virtual void loadTrack(StorageOperationData *od){}
	virtual void saveViaRenderer(StorageOperationData *od){}

	virtual void loadSong(StorageOperationData *od);
	virtual void saveSong(StorageOperationData *od);

	void make_consistent(Song *s);

	File *f;
	Song *song;
	StorageOperationData *od;
};

#endif /* FORMATNAMI_H_ */
