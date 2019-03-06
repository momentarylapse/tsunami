/*
 * StorageOperationData.h
 *
 *  Created on: 09.06.2015
 *      Author: michi
 */

#ifndef SRC_STORAGE_STORAGEOPERATIONDATA_H_
#define SRC_STORAGE_STORAGEOPERATIONDATA_H_

#include "../lib/base/base.h"

class Song;
class Progress;
namespace hui{
	class Window;
}
class AudioBuffer;
class Track;
class TrackLayer;
class Storage;
class Format;
class Port;
class Tag;
class Storage;
class Session;
enum class SampleFormat;

class StorageOperationData
{
public:
	StorageOperationData(Storage *storage, Format *format, Song *s, TrackLayer *l, const string &filename, const string &message, hui::Window *win);
	virtual ~StorageOperationData();

	void info(const string &message);
	void warn(const string &message);
	void error(const string &message);

	void set(float t);
	void set(const string &str, float t);

	int get_num_samples();

	void suggest_samplerate(int samplerate);
	void suggest_channels(int channels);
	void suggest_default_format(SampleFormat format);
	void suggest_tag(const string &key, const string &value);

	Storage *storage;
	Format *format;

	hui::Window *win;

	Session *session;
	Song *song;
	Progress *progress;
	string filename;
	AudioBuffer *buf;
	int channels_suggested;
	bool allow_channels_change;
	Track *track;
	TrackLayer *layer;
	Port *renderer;
	Array<Tag> tags;
	int num_samples;
	int offset;

	bool only_load_metadata;
};

#endif /* SRC_STORAGE_STORAGEOPERATIONDATA_H_ */
