/*
 * FormatWave.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatWave.h"
#include "../Tsunami.h"


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
	CFile *f = OpenFile(filename);
	if (!f)
		return;
	f->SetBinaryMode(true);
	char header[44];
	f->ReadBuffer(header, 44);
	if ((header[0] != 'R') or (header[1] != 'I') or (header[2] != 'F') or (header[3] != 'F')){
		msg_error("wave file does not start with \"RIFF\"");
		msg_db_l(1);
		return;
	}
	if (*(int*)&header[4] != f->GetSize())
		msg_write(format("wave file gives wrong size: %d  (real: %d)", *(int*)&header[4], f->GetSize()));
		// sometimes 0x2400ff7f
	if ((header[8] != 'W') or (header[9] != 'A') or (header[10] != 'V') or (header[11] != 'E') or (header[12] != 'f') or (header[13] != 'm') or (header[14] != 't') or (header[15] != ' ')){
		msg_error("\"WAVEfmt \" expected in wave file");
		msg_db_l(1);
		return;
	}
	if ((*(int*)&header[16] != 16) or (*(short*)&header[20] != 1)){
		msg_write("wave file does not have format 16/1");
		msg_db_l(1);
		return;
	}
	int channels = *(short*)&header[22];
	int freq = *(int*)&header[24];
	t->root->sample_rate = freq;
	int block_align = *(short*)&header[32];
	int bits = *(short*)&header[34];
	int byte_per_sample = (bits / 8) * channels;
	if ((header[36] != 'd') or (header[37] != 'a') or (header[38] != 't') or (header[39] != 'a')){
		msg_error("\"data\" expected in wave file");
		msg_db_l(1);
		return;
	}
	int size = *(int*)&header[40];
	if ((size > f->GetSize() - 44) or (size < 0)){
		msg_write("wave file gives wrong data size");
		size = f->GetSize() - 44;
	}
	int samples = size / byte_per_sample;
	tsunami->progress->Set(0.1f);

	int read = 0;
	int nn = 0;
	char *data = new char[WAVE_BUFFER_SIZE];
	while (read < size){
		int toread = clampi(WAVE_BUFFER_SIZE, 0, size - read);
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
			msg_error("could not read in wave file...");
			break;
		}
	}

	delete[](data);

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




