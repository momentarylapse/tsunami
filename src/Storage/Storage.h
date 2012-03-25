/*
 * Storage.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include "StorageAny.h"
#include "../lib/file/file.h"
#include "../lib/hui/hui.h"

class Storage
{
public:
	Storage();
	virtual ~Storage();

	bool Load(AudioFile *a, const string &filename);
	bool Save(AudioFile *a, const string &filename);


	bool AskOpen(CHuiWindow *win);
	bool AskSave(CHuiWindow *win);
	bool AskOpenImport(CHuiWindow *win);
	bool AskSaveExport(CHuiWindow *win);

private:
	Array<StorageAny*> format;
	string CurrentDirectory;
};

#endif /* STORAGE_H_ */
