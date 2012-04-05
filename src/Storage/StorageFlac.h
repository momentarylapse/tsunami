/*
 * StorageFlac.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef STORAGEFLAC_H_
#define STORAGEFLAC_H_

#include "StorageAny.h"

class StorageFlac: public StorageAny
{
public:
	StorageFlac();
	virtual ~StorageFlac();

	void LoadTrack(Track *t, const string &filename);
	void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);
};

#endif /* STORAGEFLAC_H_ */
