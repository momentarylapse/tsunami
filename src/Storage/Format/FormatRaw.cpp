/*
 * FormatRaw.cpp
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#include "FormatRaw.h"
#include "../../lib/math/math.h"
#include "../../Audio/SongRenderer.h"
#include "../Dialog/RawConfigDialog.h"


FormatRaw::FormatRaw() :
	Format("Raw audio data", "raw", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_READ | FLAG_WRITE)
{
}

FormatRaw::~FormatRaw()
{
}

RawConfigData GetRawConfigData(HuiWindow *win)
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
	AudioRenderer *r = od->renderer;

	File *f = FileCreate(od->filename);
	f->SetBinaryMode(true);

	for (int i=0; i<config.offset; i++)
		f->WriteByte(0);

	BufferBox buf;
	buf.resize(CHUNK_SIZE);
	int samples = r->getNumSamples();
	int done = 0;
	while (r->readResize(buf) > 0){
		string data;
		if (!buf.exports(data, config.channels, config.format))
			od->warn(_("Amplitude zu gro&s, Signal &ubersteuert."));
		od->set(float(done) / (float)samples);
		f->WriteBuffer(data.data, data.num);
		done += buf.num;
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
			throw string("can't open file");
		f->SetBinaryMode(true);
		int byte_per_sample = (format_get_bits(config.format) / 8) * config.channels;
		long long size = f->GetSize64() - config.offset;
		//int samples = size / byte_per_sample;
		od->set(0.1f);

		if (config.offset > 0)
			f->ReadBuffer(data, config.offset);

		long long read = 0;
		int nn = 0;
		int nice_buffer_size = WAVE_BUFFER_SIZE - (WAVE_BUFFER_SIZE % byte_per_sample);
		while (read < size){
			int toread = min(nice_buffer_size, size - read);
			int r = f->ReadBuffer(data, toread);
			nn ++;
			if (nn > 16){
				float perc_read = 0.1f;
				float dperc_read = 0.9f;
				od->set(perc_read + dperc_read * (float)read / (float)size);
				nn = 0;
			}
			if (r > 0){
				int dsamples = r / byte_per_sample;
				int _offset = read / byte_per_sample + od->offset;
				importData(od->track, data, config.channels, config.format, dsamples, _offset, od->level);
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



