/*
 * FormatRaw.cpp
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#include "FormatRaw.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../View/Helper/Progress.h"
#include "../../Stuff/Log.h"
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

RawConfigData GetRawConfigData()
{
	RawConfigData data;
	RawConfigDialog *dlg = new RawConfigDialog(&data, tsunami->win);
	dlg->run();
	return data;
}

void FormatRaw::saveBuffer(AudioFile *a, BufferBox *b, const string &filename)
{
	msg_db_f("write_raw_file", 1);
	tsunami->progress->set(_("exportiere raw"), 0);

	RawConfigData config = GetRawConfigData();

	string data;
	if (!b->exports(data, config.channels, config.format))
		tsunami->log->warning(_("Amplitude zu gro&s, Signal &ubersteuert."));

	CFile *f = FileCreate(filename);
	f->SetBinaryMode(true);

	for (int i=0; i<config.offset; i++)
		f->WriteByte(0);

	int size = data.num;
	for (int i=0;i<size / WAVE_BUFFER_SIZE;i++){
		tsunami->progress->set(float(i * WAVE_BUFFER_SIZE) / (float)size);
		f->WriteBuffer(&data[i * WAVE_BUFFER_SIZE], WAVE_BUFFER_SIZE);
	}
	f->WriteBuffer(&data[(size / WAVE_BUFFER_SIZE) * WAVE_BUFFER_SIZE], size & (WAVE_BUFFER_SIZE - 1));

	FileClose(f);
}

void FormatRaw::loadTrack(Track *t, const string & filename, int offset, int level)
{
	msg_db_f("load_raw_file", 1);
	tsunami->progress->set(_("lade raw"), 0);

	RawConfigData config = GetRawConfigData();

	char *data = new char[WAVE_BUFFER_SIZE];
	CFile *f = FileOpen(filename);

	try{

		if (!f)
			throw string("can't open file");
		f->SetBinaryMode(true);
		int byte_per_sample = (format_get_bits(config.format) / 8) * config.channels;
		int size = f->GetSize() - config.offset;
		int samples = size / byte_per_sample;
		tsunami->progress->set(0.1f);

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
				tsunami->progress->set(perc_read + dperc_read * (float)read / (float)size);
				nn = 0;
			}
			if (r > 0){
				int dsamples = r / byte_per_sample;
				int _offset = read / byte_per_sample + offset;
				importData(t, data, config.channels, config.format, dsamples, _offset, level);
				read += r;
			}else{
				throw string("could not read in wave file...");
			}
		}

	}catch(const string &s){
		tsunami->log->error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
}

void FormatRaw::saveAudio(AudioFile *a, const string & filename)
{
	exportAudioAsTrack(a, filename);
}



void FormatRaw::loadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO);
	loadTrack(t, filename);
}




