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

class Storage
{
public:
	Storage();
	virtual ~Storage();

	bool Load(AudioFile *a, const string &filename);
	bool LoadTrack(Track *t, const string &filename, int offset = 0, int level = 0);
	bool Save(AudioFile *a, const string &filename);
	bool Export(AudioFile *a, const string &filename);

	bool AskByFlags(HuiWindow *win, const string &title, bool save, int flags);

	bool AskOpen(HuiWindow *win);
	bool AskSave(HuiWindow *win);
	bool AskOpenImport(HuiWindow *win);
	bool AskSaveExport(HuiWindow *win);

private:
	Array<Format*> format;
	string CurrentDirectory;
};

#endif /* STORAGE_H_ */
