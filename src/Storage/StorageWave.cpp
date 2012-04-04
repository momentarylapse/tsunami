/*
 * StorageWave.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "StorageWave.h"
#include "../Tsunami.h"

StorageWave::StorageWave() :
	StorageAny("wav", FLAG_SINGLE_TRACK)
{
}

StorageWave::~StorageWave()
{
}

void StorageWave::SaveBuffer(AudioFile *a, BufferBox *b, const string &filename)
{
	msg_db_r("write_wave_file", 1);
	tsunami->progress->Set(_("exportiere wave"), 0);

	Array<short> buf16;
	b->get_16bit_buffer(buf16);
	char *data = (char*)buf16.data;

	CFile *f = CreateFile(filename);
	f->SetBinaryMode(true);

	f->WriteBuffer("RIFF", 4);
	f->WriteInt(b->num * 4 + 44);
	f->WriteBuffer("WAVEfmt ",8);
	f->WriteInt(16); // chunk size (fmt)
	f->WriteWord(1); // version
	f->WriteWord(2); // channels
	f->WriteInt(a->sample_rate);
	f->WriteInt(a->sample_rate * 4); // bytes per sec
	f->WriteWord(4); // block align
	f->WriteWord(16); // bits per sample
	f->WriteBuffer("data", 4);
	f->WriteInt(b->num * 4);

	/*ProgressStatus(_("exportiere wave"), 0.5f);
	f->WriteBuffer((char*)PVData,w->length*4);*/
	int size = b->num * 4;
	int chunk_size = 1 << 15;
	for (int i=0;i<size / chunk_size;i++){
		tsunami->progress->Set(float(i * chunk_size) / (float)size);
		f->WriteBuffer(&data[i * chunk_size], chunk_size);
	}
	f->WriteBuffer(&data[(size / chunk_size) * chunk_size], size & (chunk_size - 1));

	FileClose(f);
	msg_db_l(1);
}



void StorageWave::LoadTrack(Track *t, const string & filename)
{
}

void StorageWave::SaveAudio(AudioFile *a, const string & filename)
{
}



void StorageWave::LoadAudio(AudioFile *a, const string & filename)
{
}




