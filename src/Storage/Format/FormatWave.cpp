/*
 * FormatWave.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatWave.h"
#include "../../Tsunami.h"
#include "../../View/Helper/Progress.h"
#include "../../Stuff/Log.h"
#include "../../lib/math/math.h"


const int WAVE_BUFFER_SIZE = 1 << 15;

FormatWave::FormatWave() :
	Format("Wave", "wav,wave", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_READ | FLAG_WRITE)
{
}

FormatWave::~FormatWave()
{
}

void FormatWave::saveBuffer(AudioFile *a, BufferBox *b, const string &filename)
{
	msg_db_r("write_wave_file", 1);
	tsunami->progress->set(_("exportiere wave"), 0);

	Array<short> buf16;
	if (!b->get_16bit_buffer(buf16))
		tsunami->log->error(_("Amplitude zu gro&s, Signal &ubersteuert."));
	char *data = (char*)buf16.data;

	CFile *f = FileCreate(filename);
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
		tsunami->progress->set(float(i * WAVE_BUFFER_SIZE) / (float)size);
		f->WriteBuffer(&data[i * WAVE_BUFFER_SIZE], WAVE_BUFFER_SIZE);
	}
	f->WriteBuffer(&data[(size / WAVE_BUFFER_SIZE) * WAVE_BUFFER_SIZE], size & (WAVE_BUFFER_SIZE - 1));

	FileClose(f);
	msg_db_l(1);
}

static string read_chunk_name(CFile *f)
{
	string s;
	s.resize(4);
	*(int*)s.data = f->ReadInt();
	return s;
}

void FormatWave::loadTrack(Track *t, const string & filename, int offset, int level)
{
	msg_db_r("load_wave_file", 1);
	tsunami->progress->set(_("lade wave"), 0);

	char *data = new char[WAVE_BUFFER_SIZE];
	CFile *f = FileOpen(filename);

	try{

	if (!f)
		throw string("can't open file");
	f->SetBinaryMode(true);
	if (read_chunk_name(f) != "RIFF")
		throw string("wave file does not start with \"RIFF\"");
	int stated_file_size = f->ReadInt();
	int real_file_size = f->GetSize();
	if (stated_file_size > real_file_size)
		tsunami->log->warning(format("wave file gives wrong size: %d  (real: %d)", stated_file_size, real_file_size));
		// sometimes 0x2400ff7f
	if (read_chunk_name(f) != "WAVE")
		throw string("\"WAVE\" expected in wave file");


	int format_code, channels, freq, block_align, bits, byte_per_sample;
	bool fmt_chunk_read = false;
	bool data_chunk_read = false;

	// read chunks
	while (f->GetPos() < real_file_size - 8){
		string chunk_name = read_chunk_name(f);
		int chunk_size = f->ReadInt();

		if (chunk_name == "fmt "){
			if ((chunk_size != 16) && (chunk_size != 18) && (chunk_size != 40))
				throw format("wave file gives header size %d (16, 18 or 40 expected)", chunk_size);
			char header[16];
			f->ReadBuffer(header, 16);
			for (int i=0;i<chunk_size-16;i++)
				f->ReadByte();

			format_code = *(short*)&header[0];
			if ((format_code != 1) && (format_code != 3))
				throw format("wave file has format %d (1 or 3 expected)", format_code);
			channels = *(short*)&header[2];
			freq = *(int*)&header[4];
			t->root->sample_rate = freq;
			block_align = *(short*)&header[12];
			bits = *(short*)&header[14];
			byte_per_sample = (bits / 8) * channels;

			fmt_chunk_read = true;
		}else if (chunk_name == "data"){
			if (!fmt_chunk_read)
				throw string("\"data\" chunk but no preceeding \"fmt\" chunk");


			if ((chunk_size > real_file_size - 44) or (chunk_size < 0)){
				tsunami->log->warning(format("wave file gives wrong data size (given: %d,  by file size: %d)", chunk_size, real_file_size));
				chunk_size = real_file_size - f->GetPos();
			}

			int samples = chunk_size / byte_per_sample;
			tsunami->progress->set(0.1f);

			int read = 0;
			int nn = 0;
			int nice_buffer_size = WAVE_BUFFER_SIZE - (WAVE_BUFFER_SIZE % byte_per_sample);
			while (read < chunk_size){
				int toread = clampi(nice_buffer_size, 0, chunk_size - read);
				int r = f->ReadBuffer(data, toread);
				nn ++;
				if (nn > 16){
					float perc_read = 0.1f;
					float dperc_read = 0.9f;
					tsunami->progress->set(perc_read + dperc_read * (float)read / (float)chunk_size);
					nn = 0;
				}
				if (r > 0){
					int dsamples = r / byte_per_sample;
					int _offset = read / byte_per_sample + offset;
					importData(t, data, channels, format_for_bits(bits), dsamples, _offset, level);
					read += r;
				}else{
					throw string("could not read in wave file...");
				}
			}
			break;
		}else{
			tsunami->log->warning("unhandled wave chunk: " + chunk_name);

			for (int i=0;i<chunk_size;i++)
				f->ReadByte();
		}
	}

	}catch(const string &s){
		tsunami->log->error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
	msg_db_l(1);
}

void FormatWave::saveAudio(AudioFile *a, const string & filename)
{
	exportAudioAsTrack(a, filename);
}



void FormatWave::loadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO);
	loadTrack(t, filename);
}




