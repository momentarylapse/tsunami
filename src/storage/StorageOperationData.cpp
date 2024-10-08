/*
 * StorageOperationData.cpp
 *
 *  Created on: 09.06.2015
 *      Author: michi
 */

#include "StorageOperationData.h"
#include "Storage.h"
#include "../view/helper/Progress.h"
#include "../view/TsunamiWindow.h"
#include "../Session.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/audio/AudioBuffer.h"
#include "../data/Song.h"

namespace tsunami {

static const float PROGRESS_UPDATE_DT = 0.1f;

StorageOperationData::StorageOperationData(Session *_session, Format *_format, const Path &_filename) {
	session = _session;
	win = session->win.get();
	storage = session->storage.get();
	format = _format;
	song = session->song.get();
	filename = _filename;
	channels_suggested = 2;
	allow_channels_change = false;
	layer = nullptr;
	track = nullptr;
	buf = nullptr;

	offset = 0;
	renderer = nullptr;
	num_samples = 0;
	only_load_metadata = false;
	errors_encountered = false;


	try {
		auto x = Any::parse("{" + Storage::options_in + "}");
		for (string &k: x.keys())
			parameters.dict_set(k, x[k]);
	} catch (Exception &e) {
		session->e(::format("options '%s':  %s", Storage::options_in, e.message()));
	}
	Storage::options_in = "";
}

StorageOperationData::~StorageOperationData() {
}

void StorageOperationData::start_progress(const string &message) {
	if (session->storage->allow_gui)
		progress = new Progress(message, win);
}

void StorageOperationData::set_layer(TrackLayer *l) {
	layer = l;
	track = layer->track;
	song = layer->track->song;
	channels_suggested = layer->channels;
}

void StorageOperationData::info(const string& message) {
	session->i(message);
}

void StorageOperationData::warn(const string& message) {
	session->w(message);
}

void StorageOperationData::error(const string& message) {
	session->e(message);
	errors_encountered = true;
}

void StorageOperationData::set(float t) {
	if (timer.peek() < PROGRESS_UPDATE_DT)
		return;
	timer.get();
	if (progress)
		progress->set(t);
}

void StorageOperationData::set(const string &str, float t) {
	if (progress)
		progress->set(str, t);
}

int StorageOperationData::get_num_samples() {
	if (buf)
		return buf->length;
	if (renderer)
		return num_samples;
	return 0;
}

void StorageOperationData::suggest_samplerate(int samplerate) {
	// TODO
	if (track->get_index() == 0)
		song->set_sample_rate(samplerate);
}

void StorageOperationData::suggest_channels(int channels) {
	if (allow_channels_change) {
		track->set_channels(channels);
		channels_suggested = channels;
	}
}

void StorageOperationData::suggest_default_format(SampleFormat format) {
	if (track->get_index() == 0)
		song->set_default_format(format);
}

void StorageOperationData::suggest_tag(const string &key, const string &value) {
	//if (track->get_index() == 0)
		song->add_tag(key, value);
}

}
