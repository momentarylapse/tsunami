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
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Data/Song.h"

StorageOperationData::StorageOperationData(Storage *_storage, Format *_format, Song *s, TrackLayer *l, const string &_filename, const string &message, hui::Window *_win)
{
	win = _win;
	storage = _storage;
	session = storage->session;
	format = _format;
	song = s;
	filename = _filename;
	progress = new Progress(message, win);
	channels_suggested = 2;
	allow_channels_change = false;
	layer = l;
	track = nullptr;
	if (l){
		track = l->track;
		channels_suggested = l->channels;
	}
	buf = nullptr;

	offset = 0;
	renderer = nullptr;
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

void StorageOperationData::suggest_samplerate(int samplerate)
{
	// TODO
	if (track->get_index() == 0)
		song->set_sample_rate(samplerate);
}

void StorageOperationData::suggest_channels(int channels)
{
	if (allow_channels_change){
		track->set_channels(channels);
		channels_suggested = channels;
	}
}

void StorageOperationData::suggest_default_format(SampleFormat format)
{
	if (track->get_index() == 0)
		song->set_default_format(format);
}

void StorageOperationData::suggest_tag(const string &key, const string &value)
{
	//if (track->get_index() == 0)
		song->add_tag(key, value);
}
