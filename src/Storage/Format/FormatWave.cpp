/*
 * FormatWave.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatWave.h"

#include "../../Module/Port/AudioPort.h"
#include "../../Session.h"
#include "../../lib/math/math.h"


FormatDescriptorWave::FormatDescriptorWave() :
	FormatDescriptor("Wave", "wav,wave", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::READ | Flag::WRITE)
{
}


void FormatWave::saveViaRenderer(StorageOperationData *od)
{
	AudioPort *r = od->renderer;

	SampleFormat format = SAMPLE_FORMAT_16;
	if (od->song)
		format = od->song->default_format;
	if (format == SAMPLE_FORMAT_32_FLOAT)
		format = SAMPLE_FORMAT_32;
	int bit_depth = format_get_bits(format);
	int channels = od->channels_suggested;
	msg_write(channels);
	int bytes_per_sample = bit_depth / 8 * channels;
	int samples = od->num_samples;

	File *f = FileCreate(od->filename);

	f->write_buffer("RIFF", 4);
	f->write_int(samples * bytes_per_sample + 44);
	f->write_buffer("WAVEfmt ",8);
	f->write_int(16); // chunk size (fmt)
	f->write_word(1); // version
	f->write_word(channels); // channels
	f->write_int(od->session->sample_rate());
	f->write_int(od->session->sample_rate() * bytes_per_sample); // bytes per sec
	f->write_word(4); // block align
	f->write_word(bit_depth);
	f->write_buffer("data", 4);
	f->write_int(samples * bytes_per_sample);

	AudioBuffer buf(CHUNK_SIZE, channels);
	int done = 0;
	int samples_read;
	while ((samples_read = r->read(buf)) > 0){
		buf.resize(samples_read);
		string data;
		if (!buf.exports(data, channels, SAMPLE_FORMAT_16))
			od->warn(_("Amplitude too large, signal distorted."));

		od->set(float(done) / (float)samples);
		f->write_buffer(data);

		done += buf.length;
	}

	FileClose(f);
}

static string read_chunk_name(File *f)
{
	string s;
	s.resize(4);
	*(int*)s.data = f->read_int();
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
	Track *t = od->track;

	char *data = new char[CHUNK_SIZE];
	File *f = NULL;

	try{
		f = FileOpen(od->filename);

	if (read_chunk_name(f) != "RIFF")
		throw Exception("wave file does not start with \"RIFF\"");
	int stated_file_size = f->read_int();
	int real_file_size = f->get_size();
	if (stated_file_size > real_file_size)
		od->warn(format("wave file gives wrong size: %d  (real: %d)", stated_file_size, real_file_size));
		// sometimes 0x2400ff7f
	if (read_chunk_name(f) != "WAVE")
		throw Exception("\"WAVE\" expected in wave file");


	int format_code, channels, freq, block_align, bits, byte_per_sample;
	bool fmt_chunk_read = false;
	SampleFormat format;

	// read chunks
	while (f->get_pos() < real_file_size - 8){
		string chunk_name = read_chunk_name(f);
		int chunk_size = f->read_int();

		if (chunk_name == "fmt "){
			if ((chunk_size != 16) and (chunk_size != 18) and (chunk_size != 40))
				throw Exception(::format("wave file gives header size %d (16, 18 or 40 expected)", chunk_size));
			char header[16];
			f->read_buffer(header, 16);
			for (int i=0;i<chunk_size-16;i++)
				f->read_byte();

			format_code = *(short*)&header[0];
			if ((format_code != 1) and (format_code != 3))
				throw Exception(::format("wave file has format %d (1 or 3 expected)", format_code));
			channels = *(short*)&header[2];
			freq = *(int*)&header[4];
			t->song->sample_rate = freq;
			block_align = *(short*)&header[12];
			bits = *(short*)&header[14];
			byte_per_sample = (bits / 8) * channels;

			format = format_for_bits(bits);
			od->suggest_samplerate(freq);
			od->suggest_channels(channels);
			if (t->get_index() == 0)
				t->song->setDefaultFormat(format);

			fmt_chunk_read = true;
		}else if (chunk_name == "data"){
			if (!fmt_chunk_read)
				throw Exception("\"data\" chunk but no preceeding \"fmt\" chunk");


			if ((chunk_size > real_file_size - 44) or (chunk_size < 0)){
				od->warn(::format("wave file gives wrong data size (given: %d,  by file size: %d)", chunk_size, real_file_size));
				chunk_size = real_file_size - f->get_pos();
			}

			int samples = chunk_size / byte_per_sample;
			od->set(0.1f);

			int read = 0;
			int nn = 0;
			int nice_buffer_size = CHUNK_SIZE - (CHUNK_SIZE % byte_per_sample);
			while (read < chunk_size){
				int toread = clampi(nice_buffer_size, 0, chunk_size - read);
				int r = f->read_buffer(data, toread);
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
					importData(od->layer, data, channels, format, dsamples, _offset);
					read += r;
				}else{
					throw Exception("could not read in wave file...");
				}
			}
			break;
		}else if (chunk_name == "LIST"){

			// tags...
			string buf;
			buf.resize(chunk_size);
			f->read_buffer(buf.data, buf.num);
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

					od->suggest_tag(tag_from_wave(key), value);
				}
			}

		}else{
			od->warn("unhandled wave chunk: " + chunk_name);

			for (int i=0;i<chunk_size;i++)
				f->read_byte();
		}
	}

	}catch(Exception &e){
		od->error(e.message());
	}

	delete[](data);

	if (f)
		FileClose(f);
}




