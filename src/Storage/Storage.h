/*
 * Storage.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include "Format.h"
#include "../lib/file/file.h"
#include "../lib/hui/hui.h"

class Range;

class Storage
{
public:
	Storage();
	virtual ~Storage();

	bool load(AudioFile *a, const string &filename);
	bool loadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	bool loadBufferBox(AudioFile *a, BufferBox *buf, const string &filename);
	bool save(AudioFile *a, const string &filename);
	bool _export(AudioFile *a, const Range &r, const string &filename);

	bool askByFlags(HuiWindow *win, const string &title, bool save, int flags);

	bool askOpen(HuiWindow *win);
	bool askSave(HuiWindow *win);
	bool askOpenImport(HuiWindow *win);
	bool askSaveExport(HuiWindow *win);

private:
	Array<Format*> format;
	string CurrentDirectory;
};

#endif /* STORAGE_H_ */
