/*
 * StorageOperationData.cpp
 *
 *  Created on: 09.06.2015
 *      Author: michi
 */

#include "StorageOperationData.h"
#include "../View/Helper/Progress.h"

StorageOperationData::StorageOperationData(Song *s, Track *t, BufferBox *b, const string &_filename, const string &message, HuiWindow *win)
{
	song = s;
	filename = _filename;
	progress = new Progress(message, win);
	buf = b;
	track = t;
	offset = 0;
	level = 0;
}

StorageOperationData::~StorageOperationData()
{
	delete(progress);
}

