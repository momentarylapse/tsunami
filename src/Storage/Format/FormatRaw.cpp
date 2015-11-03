/*
 * FormatRaw.cpp
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#include "FormatRaw.h"
#include "../../lib/math/math.h"
#include "../Dialog/RawConfigDialog.h"


const int WAVE_BUFFER_SIZE = 1 << 15;

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

void FormatRaw::saveBuffer(StorageOperationData *od)
{
	RawConfigData config = GetRawConfigData(od->win);

	string data;
	if (!od->buf->exports(data, config.channels, config.format))
		od->warn(_("Amplitude zu gro&s, Signal &ubersteuert."));

	File *f = FileCreate(od->filename);
	f->SetBinaryMode(true);

	for (int i=0; i<config.offset; i++)
		f->WriteByte(0);

	int size = data.num;
	for (int i=0;i<size / WAVE_BUFFER_SIZE;i++){
		od->set(float(i * WAVE_BUFFER_SIZE) / (float)size);
		f->WriteBuffer(&data[i * WAVE_BUFFER_SIZE], WAVE_BUFFER_SIZE);
	}
	f->WriteBuffer(&data[(size / WAVE_BUFFER_SIZE) * WAVE_BUFFER_SIZE], size & (WAVE_BUFFER_SIZE - 1));

	FileClose(f);
}

void FormatRaw::loadTrack(StorageOperationData *od)
{
	RawConfigData config = GetRawConfigData(od->win);

	char *data = new char[WAVE_BUFFER_SIZE];
	File *f = FileOpen(od->filename);

	try{

		if (!f)
			throw string("can't open file");
		f->SetBinaryMode(true);
		int byte_per_sample = (format_get_bits(config.format) / 8) * config.channels;
		int size = f->GetSize() - config.offset;
		int samples = size / byte_per_sample;
		od->set(0.1f);

		if (config.offset > 0)
			f->ReadBuffer(data, config.offset);

		int read = 0;
		int nn = 0;
		int nice_buffer_size = WAVE_BUFFER_SIZE - (WAVE_BUFFER_SIZE % byte_per_sample);
		while (read < size){
			int toread = clampi(nice_buffer_size, 0, size - read);
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
				throw string("could not read in wave file...");
			}
		}

	}catch(const string &s){
		od->error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
}

void FormatRaw::saveSong(StorageOperationData *od)
{
	exportAsTrack(od);
}



void FormatRaw::loadSong(StorageOperationData *od)
{
	od->track = od->song->addTrack(Track::TYPE_AUDIO);
	loadTrack(od);
}




