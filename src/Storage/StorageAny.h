/*
 * StorageAny.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGEANY_H_
#define STORAGEANY_H_

#include "../Data/Track.h"
#include "../Data/AudioFile.h"

class StorageAny
{
public:
	StorageAny(const string &_extension, int _flags);
	virtual ~StorageAny();
	bool CanHandle(const string &_extension);

	void ImportData(Track *t, void *data, int channels, int bits, int samples);

	virtual void LoadTrack(Track *t, const string &filename) = 0;
	virtual void SaveTrack(Track *t, const string &filename) = 0;

	virtual void LoadAudio(AudioFile *a, const string &filename) = 0;
	virtual void SaveAudio(AudioFile *a, const string &filename) = 0;

	string extension;
	int flags;

	enum{
		FLAG_SINGLE_TRACK = 1,
		FLAG_TAGS = 2,
		FLAG_FX = 4,
		FLAG_MULTITRACK = 8,
		FLAG_SUBS = 16,
	};
};

#endif /* STORAGEANY_H_ */
