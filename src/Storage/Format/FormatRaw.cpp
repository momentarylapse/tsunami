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
#include "../../Session.h"
#include "../../TsunamiWindow.h"
#include "../Dialog/RawConfigDialog.h"


FormatDescriptorRaw::FormatDescriptorRaw() :
	FormatDescriptor("Raw audio data", "raw", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::READ | Flag::WRITE)
{
}

RawConfigData GetRawConfigData(Session *session)
{
	RawConfigData data;
	if (session->storage_options != ""){
		auto x = session->storage_options.explode(":");
		if (x.num >= 3){
			data.offset = 0;
			data.format = SampleFormat::SAMPLE_FORMAT_16;
			if (x[0] == "f32")
				data.format = SampleFormat::SAMPLE_FORMAT_32_FLOAT;
			data.channels = x[1]._int();
			data.sample_rate = x[2]._int();
			return data;
		}
	}
	RawConfigDialog *dlg = new RawConfigDialog(&data, session->win);
	dlg->run();
	return data;
}

void FormatRaw::save_via_renderer(StorageOperationData *od)
{
	RawConfigData config = GetRawConfigData(od->session);
	AudioPort *r = od->renderer;

	File *f = FileCreate(od->filename);

	for (int i=0; i<config.offset; i++)
		f->write_byte(0);

	AudioBuffer buf;
	buf.resize(CHUNK_SIZE);
	int samples = od->num_samples;
	int done = 0;
	int samples_read;
	while ((samples_read = r->read(buf)) > 0){
		string data;
		buf.resize(samples_read);
		if (!buf.exports(data, config.channels, config.format))
			od->warn(_("Amplitude too large, signal distorted."));
		od->set(float(done) / (float)samples);
		f->write_buffer(data);
		done += buf.length;
	}

	FileClose(f);
}

void FormatRaw::load_track(StorageOperationData *od)
{
	RawConfigData config = GetRawConfigData(od->session);

	char *data = new char[CHUNK_SIZE];
	File *f = nullptr;

	od->suggest_samplerate(config.sample_rate);
	od->suggest_channels(config.channels);

	try{
		f = FileOpen(od->filename);

		int byte_per_sample = (format_get_bits(config.format) / 8) * config.channels;
		long long size = f->get_size64() - config.offset;
		//int samples = size / byte_per_sample;

		if (config.offset > 0)
			f->read_buffer(data, config.offset);

		long long read = 0;
		int nn = 0;
		int nice_buffer_size = CHUNK_SIZE - (CHUNK_SIZE % byte_per_sample);
		while (read < size){
			int toread = (int)min((long long)nice_buffer_size, size - read);
			int r = f->read_buffer(data, toread);
			nn ++;
			if (nn > 16){
				od->set((float)read / (float)size);
				nn = 0;
			}
			if (r > 0){
				int dsamples = r / byte_per_sample;
				int _offset = read / byte_per_sample + od->offset;
				importData(od->layer, data, config.channels, config.format, dsamples, _offset);
				read += r;
			}else{
				throw Exception("could not read in raw file...");
			}
		}

	}catch(Exception &e){
		od->error(e.message());
	}

	delete[](data);

	if (f)
		FileClose(f);
}



