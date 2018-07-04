/*
 * StorageOperationData.cpp
 *
 *  Created on: 09.06.2015
 *      Author: michi
 */

#include "StorageOperationData.h"
#include "Storage.h"

#include "../View/Helper/Progress.h"
#include "../Session.h"
#include "../Module/Port/AudioPort.h"
#include "../Data/Track.h"
#include "../Data/Audio/AudioBuffer.h"

StorageOperationData::StorageOperationData(Storage *_storage, Format *_format, Song *s, TrackLayer *l, AudioBuffer *b, const string &_filename, const string &message, hui::Window *_win)
{
	win = _win;
	storage = _storage;
	session = storage->session;
	format = _format;
	song = s;
	filename = _filename;
	progress = new Progress(message, win);
	buf = b;
	layer = l;
	track = NULL;
	if (l)
		track = l->track;
	offset = 0;
	renderer = NULL;
	num_samples = 0;
	only_load_metadata = false;
}

StorageOperationData::~StorageOperationData()
{
	delete(progress);
}

void StorageOperationData::info(const string& message)
{
	session->i(filename + ": " + message);
}

void StorageOperationData::warn(const string& message)
{
	session->w(filename + ": " + message);
}

void StorageOperationData::error(const string& message)
{
	session->e(filename + ": " + message);
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
