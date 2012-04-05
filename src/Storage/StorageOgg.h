/*
 * StorageOgg.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef STORAGEOGG_H_
#define STORAGEOGG_H_

#include "StorageAny.h"

class StorageOgg: public StorageAny
{
public:
	StorageOgg();
	virtual ~StorageOgg();

	void LoadTrack(Track *t, const string &filename);
	void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);
};

#endif /* STORAGEOGG_H_ */
