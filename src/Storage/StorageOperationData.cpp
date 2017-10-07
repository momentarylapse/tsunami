/*
 * StorageOperationData.cpp
 *
 *  Created on: 09.06.2015
 *      Author: michi
 */

#include "StorageOperationData.h"

#include "../Audio/Source/AudioSource.h"
#include "../View/Helper/Progress.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include "../Audio/AudioBuffer.h"

StorageOperationData::StorageOperationData(Storage *_storage, Format *_format, Song *s, Track *t, AudioBuffer *b, const string &_filename, const string &message, hui::Window *_win)
{
	win = _win;
	storage = _storage;
	format = _format;
	song = s;
	filename = _filename;
	progress = new Progress(message, win);
	buf = b;
	track = t;
	offset = 0;
	layer = 0;
	renderer = NULL;
	num_samples = 0;
}

StorageOperationData::~StorageOperationData()
{
	delete(progress);
}

void StorageOperationData::info(const string& message)
{
	tsunami->log->info(filename + ": " + message);
}

void StorageOperationData::warn(const string& message)
{
	tsunami->log->warn(filename + ": " + message);
}

void StorageOperationData::error(const string& message)
{
	tsunami->log->error(filename + ": " + message);
}

void StorageOperationData::set(float t)
{
	progress->set(t);
}

void StorageOperationData::set(const string &str, float t)
{
	progress->set(str, t);
}

int StorageOperationData::get_num_samples()
{
	if (buf)
		return buf->length;
	if (renderer)
		return num_samples;
	return 0;
}
