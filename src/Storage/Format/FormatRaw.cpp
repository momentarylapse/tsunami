/*
 * FormatRaw.cpp
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#include "FormatRaw.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../../lib/math/math.h"
#include "../../Data/base.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Session.h"
#include "../../TsunamiWindow.h"
#include "../Dialog/RawConfigDialog.h"


FormatDescriptorRaw::FormatDescriptorRaw() :
	FormatDescriptor("Raw audio data", "raw", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::READ | Flag::WRITE) {}


bool FormatRaw::get_parameters(StorageOperationData *od, bool save) {
	od->parameters.set("format", i2s((int)SampleFormat::SAMPLE_FORMAT_32_FLOAT));
	od->parameters.set("channels", "2");
	od->parameters.set("samplerate", i2s(od->session->sample_rate()));
	od->parameters.set("offset", "0");
	
	if (od->session->storage_options != "") {
		auto x = od->session->storage_options.explode(":");
		if (x.num >= 3) {
			od->parameters.set("offset", "0");
			od->parameters.set("format", i2s((int)SampleFormat::SAMPLE_FORMAT_16));
			if (x[0] == "f32")
				od->parameters.set("format", i2s((int)SampleFormat::SAMPLE_FORMAT_32_FLOAT));
				
			od->parameters.set("channels", x[1]);
			od->parameters.set("sample_rate", x[2]);
			return true;
		}
	}
	auto *dlg = new RawConfigDialog(od, od->session->win);
	dlg->run();
	bool ok = dlg->ok;
	delete dlg;
	return ok;
}

void FormatRaw::save_via_renderer(StorageOperationData *od) {
	Port *r = od->renderer;

	File *f = FileCreate(od->filename);
	
	int offset = od->parameters["offset"]._int();
	int channels = od->parameters["channels"]._int();
	auto format = (SampleFormat)od->parameters["format"]._int();
	

	for (int i=0; i<offset; i++)
		f->write_byte(0);

	AudioBuffer buf;
	buf.resize(CHUNK_SIZE);
	int samples = od->num_samples;
	int done = 0;
	int samples_read;
	while ((samples_read = r->read_audio(buf)) > 0) {
		string data;
		buf.resize(samples_read);
		if (!buf.exports(data, channels, format))
			od->warn(_("Amplitude too large, signal distorted."));
		od->set(float(done) / (float)samples);
		f->write_buffer(data);
		done += buf.length;
	}

	FileClose(f);
}

void FormatRaw::load_track(StorageOperationData *od) {
	int offset = od->parameters["offset"]._int();
	int channels = od->parameters["channels"]._int();
	int sample_rate = od->parameters["samplerate"]._int();
	auto format = (SampleFormat)od->parameters["format"]._int();

	char *data = new char[CHUNK_SIZE];
	File *f = nullptr;

	od->suggest_samplerate(sample_rate);
	od->suggest_channels(channels);

	try {
		f = FileOpen(od->filename);

		int byte_per_sample = (format_get_bits(format) / 8) * channels;
		long long size = f->get_size64() - offset;
		//int samples = size / byte_per_sample;

		if (offset > 0)
			f->read_buffer(data, offset);

		long long read = 0;
		int nn = 0;
		int nice_buffer_size = 10000;//CHUNK_SIZE - (CHUNK_SIZE % byte_per_sample);
		while (read < size) {
			int toread = (int)min((long long)nice_buffer_size, size - read);
			int r = f->read_buffer(data, toread);
			nn ++;
			if (nn > 16) {
				od->set((float)read / (float)size);
				nn = 0;
			}
			if (r > 0) {
				int dsamples = r / byte_per_sample;
				int _offset = read / byte_per_sample + od->offset;
				import_data(od->layer, data, channels, format, dsamples, _offset);
				read += r;
			} else {
				throw Exception("could not read in raw file...");
			}
		}

	} catch(Exception &e) {
		od->error(e.message());
	}

	delete[](data);

	if (f)
		FileClose(f);
}



