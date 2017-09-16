/*
 * Storage.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include "../lib/file/file.h"
#include "../lib/hui/hui.h"

class Range;
class Format;
class FormatDescriptor;
class Song;
class Track;
class AudioBuffer;
class AudioSource;
class StorageOperationData;
class Tag;

class Storage
{
public:
	Storage();
	virtual ~Storage();

	bool load(Song *a, const string &filename);
	bool loadTrack(Track *t, const string &filename, int offset = 0, int layer = 0);
	bool loadBufferBox(Song *a, AudioBuffer *buf, const string &filename);
	bool saveViaRenderer(AudioSource *r, const string &filename, int num_samples, const Array<Tag> &tags);
	bool save(Song *a, const string &filename);

	bool askByFlags(hui::Window *win, const string &title, int flags);

	bool askOpen(hui::Window *win);
	bool askSave(hui::Window *win);
	bool askOpenImport(hui::Window *win);
	bool askSaveExport(hui::Window *win);

	FormatDescriptor *getFormat(const string &ext, int flags);

//private:
	Array<FormatDescriptor*> formats;
	string current_directory;
};

#endif /* STORAGE_H_ */
