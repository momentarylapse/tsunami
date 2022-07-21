/*
 * FormatRaw.cpp
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#include "FormatRaw.h"
#include "../dialog/RawConfigDialog.h"
#include "../../module/audio/SongRenderer.h"
#include "../../lib/math/math.h"
#include "../../lib/os/file.h"
#include "../../lib/os/formatter.h"
#include "../../data/base.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../Session.h"
#include "../../TsunamiWindow.h"


FormatDescriptorRaw::FormatDescriptorRaw() :
	FormatDescriptor("Raw audio data", "raw", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::READ | Flag::WRITE) {}


bool FormatRaw::get_parameters(StorageOperationData *od, bool save) {
	// optional defaults
	if (!od->parameters.has("offset"))
		od->parameters.map_set("offset", 0);
	if (!od->parameters.has("samplerate"))
		od->parameters.map_set("samplerate", od->session->sample_rate());
	
	if (od->parameters.has("format") and od->parameters.has("channels"))
		return true;

	// mandatory defaults
	if (!od->parameters.has("format"))
		od->parameters.map_set("format", "f32");
	if (!od->parameters.has("channels"))
		od->parameters.map_set("channels", 1);


	bool ok = false;
	auto dlg = new RawConfigDialog(od, od->win);
	hui::run(dlg, [&ok,dlg] {
		ok = dlg->ok;
	});
	return ok;
}

void FormatRaw::save_via_renderer(StorageOperationData *od) {
	Port *r = od->renderer;

	auto f = new BinaryFormatter(os::fs::open(od->filename, "wb"));
	
	int offset = od->parameters["offset"]._int();
	int channels = od->parameters["channels"]._int();
	auto format = format_from_code(od->parameters["format"].str());
	

	for (int i=0; i<offset; i++)
		f->write_byte(0);

	AudioBuffer buf;
	buf.resize(CHUNK_SIZE);
	int samples = od->num_samples;
	int done = 0;
	int samples_read;
	while ((samples_read = r->read_audio(buf)) > 0) {
		bytes data;
		buf.resize(samples_read);
		if (!buf.exports(data, channels, format))
			od->warn(_("Amplitude too large, signal distorted."));
		od->set(float(done) / (float)samples);
		f->write(data);
		done += buf.length;
	}

	delete f;
}

void FormatRaw::load_track(StorageOperationData *od) {
	int offset = od->parameters["offset"]._int();
	int channels = od->parameters["channels"]._int();
	int sample_rate = od->parameters["samplerate"]._int();
	auto format = format_from_code(od->parameters["format"].str());

	bytes data;
	BinaryFormatter *f = nullptr;

	od->suggest_samplerate(sample_rate);
	od->suggest_channels(channels);

	try {
		f = new BinaryFormatter(os::fs::open(od->filename, "rb"));

		int byte_per_sample = (format_get_bits(format) / 8) * channels;
		int64 size = f->get_size() - offset;
		//int samples = size / byte_per_sample;

		if (offset > 0)
			f->read(offset);

		long long read = 0;
		int nice_buffer_size = CHUNK_SIZE - (CHUNK_SIZE % byte_per_sample);
		while (read < size) {
			int toread = (int)min((int64)nice_buffer_size, size - read);
			data = f->read(toread);
			od->set((float)read / (float)size);
			if (data.num > 0) {
				int dsamples = data.num / byte_per_sample;
				int _offset = read / byte_per_sample + od->offset;
				import_data(od->layer, &data[0], channels, format, dsamples, _offset);
				read += data.num;
			} else {
				throw Exception("could not read in raw file...");
			}
		}

	} catch(Exception &e) {
		od->error(e.message());
	}

	if (f)
		delete f;
}



