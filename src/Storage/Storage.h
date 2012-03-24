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

class Storage
{
public:
	Storage();
	virtual ~Storage();

	void Load(AudioFile *a, const string &filename);
	void Save(AudioFile *a, const string &filename);

	Array<StorageAny*> format;
};

#endif /* STORAGE_H_ */
