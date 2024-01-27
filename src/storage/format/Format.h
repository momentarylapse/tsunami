/*
 * Format.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMAT_H_
#define FORMAT_H_

#include "../StorageOperationData.h"

class StorageOperationData;
class Port;
class Format;
class Song;
namespace os::fs {
	class FileStream;
}
class Stream;
//class BinaryFormatter;
enum class SampleFormat;

const int CHUNK_SIZE = 1 << 18;
const int CHUNK_SAMPLES = 1 << 16;

class FormatDescriptor {
public:
	FormatDescriptor(const string &description, const string &extensions, int _flags);
	virtual ~FormatDescriptor(){}
	bool can_handle(const string &extension);
	bool test_compatibility(Song *s);

	virtual Format *create() = 0;

	Array<string> extensions;
	string description;
	int flags;

	enum Flag {
		SINGLE_TRACK = 1<<0,
		TAGS = 1<<1,
		FX = 1<<2,
		MULTITRACK = 1<<3,
		SAMPLES = 1<<4,
		AUDIO = 1<<5,
		MIDI = 1<<6,
		READ = 1<<7,
		WRITE = 1<<8,
	};
};

class Format {
public:
	Format();
	virtual ~Format(){}

	void import_data(TrackLayer *l, void *data, int channels, SampleFormat format, int samples, int offset);

	virtual void load_track(StorageOperationData *od) = 0;
	virtual void save_via_renderer(StorageOperationData *od) = 0;

	virtual void load_song(StorageOperationData *od);
	virtual void save_song(StorageOperationData *od);
	
	virtual bool get_parameters(StorageOperationData *od, bool save) { return true; }

	os::fs::FileStream *f;
};

#endif /* FORMAT_H_ */
