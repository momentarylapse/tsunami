/*
 * FormatRaw.cpp
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#include "FormatRaw.h"

#include "../../Audio/Source/SongRenderer.h"
#include "../../lib/math/math.h"
#include "../Dialog/RawConfigDialog.h"


FormatDescriptorRaw::FormatDescriptorRaw() :
	FormatDescriptor("Raw audio data", "raw", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_READ | FLAG_WRITE)
{
}

RawConfigData GetRawConfigData(hui::Window *win)
{
	RawConfigData data;
	RawConfigDialog *dlg = new RawConfigDialog(&data, win);
	dlg->run();
	return data;
}

void FormatRaw::saveViaRenderer(StorageOperationData *od)
{
	RawConfigData config = GetRawConfigData(od->win);
	const int CHUNK_SIZE = 1<<15;
	AudioSource *r = od->renderer;

	File *f = FileCreate(od->filename);
	if (!f){
		od->error("can not create file");
		return;
	}
	f->SetBinaryMode(true);

	for (int i=0; i<config.offset; i++)
		f->WriteByte(0);

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
		f->WriteBuffer(data.data, data.num);
		done += buf.length;
	}

	FileClose(f);
}

void FormatRaw::loadTrack(StorageOperationData *od)
{
	RawConfigData config = GetRawConfigData(od->win);

	const int WAVE_BUFFER_SIZE = 1 << 16;

	char *data = new char[WAVE_BUFFER_SIZE];
	File *f = FileOpen(od->filename);

	try{

		if (!f)
			throw string("can not open file");
		f->SetBinaryMode(true);
		int byte_per_sample = (format_get_bits(config.format) / 8) * config.channels;
		long long size = f->GetSize64() - config.offset;
		//int samples = size / byte_per_sample;

		if (config.offset > 0)
			f->ReadBuffer(data, config.offset);

		long long read = 0;
		int nn = 0;
		int nice_buffer_size = WAVE_BUFFER_SIZE - (WAVE_BUFFER_SIZE % byte_per_sample);
		while (read < size){
			int toread = (int)min((long long)nice_buffer_size, size - read);
			int r = f->ReadBuffer(data, toread);
			nn ++;
			if (nn > 16){
				od->set((float)read / (float)size);
				nn = 0;
			}
			if (r > 0){
				int dsamples = r / byte_per_sample;
				int _offset = read / byte_per_sample + od->offset;
				importData(od->track, data, config.channels, config.format, dsamples, _offset, od->layer);
				read += r;
			}else{
				throw string("could not read in raw file...");
			}
		}

	}catch(const string &s){
		od->error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
}



