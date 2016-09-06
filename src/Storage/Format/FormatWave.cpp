/*
 * FormatWave.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatWave.h"
#include "../../lib/math/math.h"
#include "../../Audio/Renderer/AudioRenderer.h"


const int WAVE_BUFFER_SIZE = 1 << 15;

FormatDescriptorWave::FormatDescriptorWave() :
	FormatDescriptor("Wave", "wav,wave", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_READ | FLAG_WRITE)
{
}


void FormatWave::saveViaRenderer(StorageOperationData *od)
{
	const int CHUNK_SIZE = 1 << 15;

	AudioRenderer *r = od->renderer;

	SampleFormat format = SAMPLE_FORMAT_16;
	if (od->song)
		format = od->song->default_format;
	if (format == SAMPLE_FORMAT_32_FLOAT)
		format = SAMPLE_FORMAT_32;
	int bit_depth = format_get_bits(format);
	int channels = 2;
	int bytes_per_sample = bit_depth / 8 * channels;
	int samples = r->getNumSamples();

	File *f = FileCreate(od->filename);
	if (!f)
		throw string("can not create file");
	f->SetBinaryMode(true);

	f->WriteBuffer("RIFF", 4);
	f->WriteInt(samples * bytes_per_sample + 44);
	f->WriteBuffer("WAVEfmt ",8);
	f->WriteInt(16); // chunk size (fmt)
	f->WriteWord(1); // version
	f->WriteWord(channels); // channels
	f->WriteInt(r->getSampleRate());
	f->WriteInt(r->getSampleRate() * bytes_per_sample); // bytes per sec
	f->WriteWord(4); // block align
	f->WriteWord(bit_depth);
	f->WriteBuffer("data", 4);
	f->WriteInt(samples * bytes_per_sample);

	BufferBox buf;
	buf.resize(CHUNK_SIZE);
	int done = 0;
	while (r->readResize(buf) > 0){
		string data;
		if (!buf.exports(data, 2, SAMPLE_FORMAT_16))
			od->warn(_("Amplitude too large, signal distorted."));

		od->set(float(done) / (float)samples);
		f->WriteBuffer(data.data, data.num);

		done += buf.length;
	}

	FileClose(f);
}

static string read_chunk_name(File *f)
{
	string s;
	s.resize(4);
	*(int*)s.data = f->ReadInt();
	return s;
}

static string tag_from_wave(const string &key)
{
	if (key == "IART")
		return "artist";
	if (key == "INAM")
		return "title";
	if (key == "ICRD")
		return "year";
	if (key == "ICMT")
		return "comment";
	if (key == "IGNR")
		return "genre";
	if (key == "IPRD")
		return "album";
	if (key == "IPRT")
		return "track";
	if (key == "ICOP")
		return "copyright";
	if (key == "ISFT")
		return "software";
	return key;
}

void FormatWave::loadTrack(StorageOperationData *od)
{
	msg_db_f("load_wave_file", 1);
	Track *t = od->track;

	char *data = new char[WAVE_BUFFER_SIZE];
	File *f = FileOpen(od->filename);

	try{

	if (!f)
		throw string("can not open file");
	f->SetBinaryMode(true);
	if (read_chunk_name(f) != "RIFF")
		throw string("wave file does not start with \"RIFF\"");
	int stated_file_size = f->ReadInt();
	int real_file_size = f->GetSize();
	if (stated_file_size > real_file_size)
		od->warn(format("wave file gives wrong size: %d  (real: %d)", stated_file_size, real_file_size));
		// sometimes 0x2400ff7f
	if (read_chunk_name(f) != "WAVE")
		throw string("\"WAVE\" expected in wave file");


	int format_code, channels, freq, block_align, bits, byte_per_sample;
	bool fmt_chunk_read = false;
	SampleFormat format;

	// read chunks
	while (f->GetPos() < real_file_size - 8){
		string chunk_name = read_chunk_name(f);
		int chunk_size = f->ReadInt();

		if (chunk_name == "fmt "){
			if ((chunk_size != 16) and (chunk_size != 18) and (chunk_size != 40))
				throw ::format("wave file gives header size %d (16, 18 or 40 expected)", chunk_size);
			char header[16];
			f->ReadBuffer(header, 16);
			for (int i=0;i<chunk_size-16;i++)
				f->ReadByte();

			format_code = *(short*)&header[0];
			if ((format_code != 1) and (format_code != 3))
				throw ::format("wave file has format %d (1 or 3 expected)", format_code);
			channels = *(short*)&header[2];
			freq = *(int*)&header[4];
			t->song->sample_rate = freq;
			block_align = *(short*)&header[12];
			bits = *(short*)&header[14];
			byte_per_sample = (bits / 8) * channels;

			format = format_for_bits(bits);
			if (t->get_index() == 0)
				t->song->setDefaultFormat(format);

			fmt_chunk_read = true;
		}else if (chunk_name == "data"){
			if (!fmt_chunk_read)
				throw string("\"data\" chunk but no preceeding \"fmt\" chunk");


			if ((chunk_size > real_file_size - 44) or (chunk_size < 0)){
				od->warn(::format("wave file gives wrong data size (given: %d,  by file size: %d)", chunk_size, real_file_size));
				chunk_size = real_file_size - f->GetPos();
			}

			int samples = chunk_size / byte_per_sample;
			od->set(0.1f);

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
					od->set(perc_read + dperc_read * (float)read / (float)chunk_size);
					nn = 0;
				}
				if (r > 0){
					int dsamples = r / byte_per_sample;
					int _offset = read / byte_per_sample + od->offset;
					importData(t, data, channels, format, dsamples, _offset, od->level);
					read += r;
				}else{
					throw string("could not read in wave file...");
				}
			}
			break;
		}else if (chunk_name == "LIST"){

			// tags...
			string buf;
			buf.resize(chunk_size);
			f->ReadBuffer(buf.data, buf.num);
			//msg_write(buf.hex());

			if (buf.head(4) == "INFO"){
				int offset = 4;
				while (offset + 8 < buf.num){
					if (buf[offset] == 0){
						offset ++;
						continue;
					}
					string key = buf.substr(offset, 4);
					int length = *(int*)buf.substr(offset + 4, 4).data;
					if (offset + 8 + length > buf.num)
						break;
					string value = buf.substr(offset + 8, length - 1);
					//msg_write(key + " : " + value.hex() + " - " + value);
					offset += 8 + length;

					t->song->tags.add(Tag(tag_from_wave(key), value));
				}
			}

		}else{
			od->warn("unhandled wave chunk: " + chunk_name);

			for (int i=0;i<chunk_size;i++)
				f->ReadByte();
		}
	}

	}catch(const string &s){
		od->error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
}




