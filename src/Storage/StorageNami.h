/*
 * StorageNami.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGENAMI_H_
#define STORAGENAMI_H_

#include "StorageAny.h"

class StorageNami : public StorageAny
{
public:
	StorageNami();
	virtual ~StorageNami();

	void LoadTrack(Track *t, const string &filename);
	void SaveTrack(Track *t, const string &filename);

	void LoadAudio(AudioFile *a, const string &filename);
	void SaveAudio(AudioFile *a, const string &filename);
};

#endif /* STORAGENAMI_H_ */
