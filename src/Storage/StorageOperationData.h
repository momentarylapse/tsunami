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
class HuiWindow;
class BufferBox;
class Track;
class Storage;
class Format;
class AudioRenderer;

class StorageOperationData
{
public:
	StorageOperationData(Storage *storage, Format *format, Song *s, Track *t, BufferBox *b, const string &filename, const string &message, HuiWindow *win);
	virtual ~StorageOperationData();

	void info(const string &message);
	void warn(const string &message);
	void error(const string &message);

	void set(float t);
	void set(const string &str, float t);

	int get_num_samples();

	Storage *storage;
	Format *format;

	HuiWindow *win;

	Song *song;
	Progress *progress;
	string filename;
	BufferBox *buf;
	Track *track;
	AudioRenderer *renderer;
	int offset;
	int layer;
};

#endif /* SRC_STORAGE_STORAGEOPERATIONDATA_H_ */
