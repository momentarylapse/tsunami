/*
 * Format.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMAT_H_
#define FORMAT_H_

#include "../../Data/Track.h"
#include "../../Data/AudioFile.h"
#include "../StorageOperationData.h"

class StorageOperationData;

class Format
{
public:
	Format(const string &description, const string &extensions, int _flags);
	virtual ~Format();
	bool canHandle(const string &extension);
	bool testFormatCompatibility(AudioFile *a);

	void importData(Track *t, void *data, int channels, SampleFormat format, int samples, int offset, int level);
	void exportAudioAsTrack(StorageOperationData *od);

	virtual void loadTrack(StorageOperationData *od) = 0;
	virtual void saveBuffer(StorageOperationData *od) = 0;

	virtual void loadAudio(StorageOperationData *od) = 0;
	virtual void saveAudio(StorageOperationData *od) = 0;

	Array<string> extensions;
	string description;
	int flags;

	enum{
		FLAG_SINGLE_TRACK = 1<<0,
		FLAG_TAGS = 1<<1,
		FLAG_FX = 1<<2,
		FLAG_MULTITRACK = 1<<3,
		FLAG_SUBS = 1<<4,
		FLAG_AUDIO = 1<<5,
		FLAG_MIDI = 1<<6,
		FLAG_READ = 1<<7,
		FLAG_WRITE = 1<<8,
	};
};

#endif /* FORMAT_H_ */
