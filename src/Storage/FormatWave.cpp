/*
 * FormatWave.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatWave.h"
#include "../Tsunami.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"


const int WAVE_BUFFER_SIZE = 1 << 15;

FormatWave::FormatWave() :
	Format("wav", FLAG_SINGLE_TRACK)
{
}

FormatWave::~FormatWave()
{
}

void FormatWave::SaveBuffer(AudioFile *a, BufferBox *b, const string &filename)
{
	msg_db_r("write_wave_file", 1);
	tsunami->progress->Set(_("exportiere wave"), 0);

	Array<short> buf16;
	if (!b->get_16bit_buffer(buf16))
		tsunami->log->Error(_("Amplitude zu gro&s, Signal &ubersteuert."));
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
	for (int i=0;i<size / WAVE_BUFFER_SIZE;i++){
		tsunami->progress->Set(float(i * WAVE_BUFFER_SIZE) / (float)size);
		f->WriteBuffer(&data[i * WAVE_BUFFER_SIZE], WAVE_BUFFER_SIZE);
	}
	f->WriteBuffer(&data[(size / WAVE_BUFFER_SIZE) * WAVE_BUFFER_SIZE], size & (WAVE_BUFFER_SIZE - 1));

	FileClose(f);
	msg_db_l(1);
}

void FormatWave::LoadTrack(Track *t, const string & filename)
{
	msg_db_r("load_wave_file", 1);
	tsunami->progress->Set(_("lade wave"), 0);

	char *data = new char[WAVE_BUFFER_SIZE];
	CFile *f = OpenFile(filename);

	try{

	if (!f)
		throw string("can't open file");
	f->SetBinaryMode(true);
	char header[44];
	f->ReadBuffer(header, 44);
	if ((header[0] != 'R') or (header[1] != 'I') or (header[2] != 'F') or (header[3] != 'F'))
		throw string("wave file does not start with \"RIFF\"");
	if (*(int*)&header[4] > f->GetSize())
		tsunami->log->Warning(format("wave file gives wrong size: %d  (real: %d)", *(int*)&header[4], f->GetSize()));
		// sometimes 0x2400ff7f
	if ((header[8] != 'W') or (header[9] != 'A') or (header[10] != 'V') or (header[11] != 'E') or (header[12] != 'f') or (header[13] != 'm') or (header[14] != 't') or (header[15] != ' '))
		throw string("\"WAVEfmt \" expected in wave file");
	int header_size = *(int*)&header[16];
	int format_code = *(short*)&header[20];
	if ((header_size != 16) && (header_size != 18) && (header_size != 40))
		throw format("wave file gives header size %d (16, 18 or 40 expected)", header_size);
	for (int i=0;i<header_size-16;i++)
		f->ReadByte();
	if ((format_code != 1) && (format_code != 3))
		throw format("wave file has format %d (1 or 3 expected)", format_code);
	int channels = *(short*)&header[22];
	int freq = *(int*)&header[24];
	t->root->sample_rate = freq;
	int block_align = *(short*)&header[32];
	int bits = *(short*)&header[34];
	int byte_per_sample = (bits / 8) * channels;
	if ((header[36] != 'd') or (header[37] != 'a') or (header[38] != 't') or (header[39] != 'a'))
		throw string("\"data\" expected in wave file");
	int size = *(int*)&header[40];
	if ((size > f->GetSize() - 44) or (size < 0)){
		tsunami->log->Warning(format("wave file gives wrong data size (given: %d,  by file size: %d)", size, f->GetSize() - 44));
		size = f->GetSize() - 44;
	}
	int samples = size / byte_per_sample;
	tsunami->progress->Set(0.1f);

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
			tsunami->progress->Set(perc_read + dperc_read * (float)read / (float)size);
			nn = 0;
		}
		if (r > 0){
			int dsamples = r / byte_per_sample;
			int offset = read / byte_per_sample;
			ImportData(t, data, channels, bits, dsamples, offset);
			read += r;
		}else{
			throw string("could not read in wave file...");
		}
	}

	}catch(const string &s){
		tsunami->log->Error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
	msg_db_l(1);
}

void FormatWave::SaveAudio(AudioFile *a, const string & filename)
{
	ExportAudioAsTrack(a, filename);
}



void FormatWave::LoadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->AddEmptyTrack();
	LoadTrack(t, filename);
}




