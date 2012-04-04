/*
 * StorageWave.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGEWAVE_H_
#define STORAGEWAVE_H_

#include "StorageAny.h"

class StorageWave: public StorageAny
{
public:
	StorageWave();
	virtual ~StorageWave();

	void LoadTrack(Track *t, const string &filename);
	void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);
};

#endif /* STORAGEWAVE_H_ */
