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

class StorageOperationData
{
public:
	StorageOperationData(Song *s, Track *t, BufferBox *b, const string &filename, const string &message, HuiWindow *win);
	virtual ~StorageOperationData();

	Song *song;
	Progress *progress;
	string filename;
	BufferBox *buf;
	Track *track;
	int offset;
	int level;
};

#endif /* SRC_STORAGE_STORAGEOPERATIONDATA_H_ */
