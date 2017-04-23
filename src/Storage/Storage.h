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
class BufferBox;
class AudioRenderer;
class StorageOperationData;

class Storage
{
public:
	Storage();
	virtual ~Storage();

	bool load(Song *a, const string &filename);
	bool loadTrack(Track *t, const string &filename, int offset = 0, int layer = 0);
	bool loadBufferBox(Song *a, BufferBox *buf, const string &filename);
	bool saveViaRenderer(AudioRenderer *r, const string &filename);
	bool save(Song *a, const string &filename);

	bool askByFlags(HuiWindow *win, const string &title, int flags);

	bool askOpen(HuiWindow *win);
	bool askSave(HuiWindow *win);
	bool askOpenImport(HuiWindow *win);
	bool askSaveExport(HuiWindow *win);

	FormatDescriptor *getFormat(const string &ext, int flags);

//private:
	Array<FormatDescriptor*> formats;
	string current_directory;
};

#endif /* STORAGE_H_ */
