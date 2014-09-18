/*
 * Format.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMAT_H_
#define FORMAT_H_

#include "../Data/Track.h"
#include "../Data/AudioFile.h"

class Format
{
public:
	Format(const string &_extensions, int _flags);
	virtual ~Format();
	bool CanHandle(const string &_extension);
	bool TestFormatCompatibility(AudioFile *a);

	void ImportData(Track *t, void *data, int channels, SampleFormat format, int samples, int offset, int level);
	void ExportAudioAsTrack(AudioFile *a, const string &filename);

	virtual void LoadTrack(Track *t, const string &filename, int offset = 0, int level = 0) = 0;
	virtual void SaveBuffer(AudioFile *a, BufferBox *b, const string &filename) = 0;

	virtual void LoadAudio(AudioFile *a, const string &filename) = 0;
	virtual void SaveAudio(AudioFile *a, const string &filename) = 0;

	Array<string> extensions;
	int flags;

	enum{
		FLAG_SINGLE_TRACK = 1,
		FLAG_TAGS = 2,
		FLAG_FX = 4,
		FLAG_MULTITRACK = 8,
		FLAG_SUBS = 16,
		FLAG_AUDIO = 32,
		FLAG_MIDI = 64,
	};
};

#endif /* FORMAT_H_ */
