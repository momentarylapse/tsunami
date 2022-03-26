/*
 * FormatMp3.cpp
 *
 *  Created on: 18.09.2014
 *      Author: michi
 */

#include "FormatMp3.h"
#include "../Storage.h"
#include "../../lib/math/math.h"
#include "../../lib/file/file.h"
#include "../../data/Track.h"
#include "../../data/Song.h"
#include "../../data/base.h"
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_FLOAT_OUTPUT
#include "../helper/minimp3.h"

FormatDescriptorMp3::FormatDescriptorMp3() :
	FormatDescriptor("Mp3", "mp3", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::TAGS | Flag::READ)
{
}

// (-_-) 4*7bit big endian
static int read_mp3_28bit(File *f) {
	unsigned char d[4];
	f->read_buffer(d, 4);

	return (d[3] & 0x7f) | ((d[2] & 0x7f) << 7) | ((d[1] & 0x7f) << 14) | ((d[0] & 0x7f) << 21);
}

static int read_32bit_be(File *f) {
	unsigned char d[4];
	f->read_buffer(d, 4);

	return d[3] | (d[2] << 8) | (d[1] << 16) | (d[0] << 24);
}

static string tag_from_mp3(const string &key) {
	if (key == "TALB")
		return "album";
	if (key == "TPE1")
		return "artist";
	if (key == "TIT2")
		return "title";
	if (key == "TRCK")
		return "track";
	if ((key == "TYER") or (key == "TDRC"))
		return "year";
	if (key == "COMM")
		return "comment";
	return key;
}

void FormatMp3::load_track(StorageOperationData *od) {
	Track *t = od->track;

	//unsigned char *data = new unsigned char[4096];
	bytes data;
	File *f = nullptr;

	try {
		f = FileOpen(od->filename);


		while(true) {
			int pos0 = f->get_pos();
			data = f->read_buffer(4);
			if ((data[0] == 0xff) and ((data[1] & 0xfe) == 0xfa)) {
				msg_write("== mp3-header ==");
				const int BIT_RATES[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
				const int FREQS[] = {44100, 48000, 32000, 0};
				bool error_correction = ((data[1] & 0x01) > 0);
				msg_write(format("correction: %d", (int)error_correction));
				int bit_rate_index = ((data[2] & 0xf0) >> 4);
				int freq_index = ((data[2] & 0x06) >> 2);
				int bit_rate = BIT_RATES[bit_rate_index];
				od->suggest_samplerate(FREQS[freq_index]);
				//t->song->sample_rate = FREQS[freq_index];
				msg_write(format("bitrate: %d", bit_rate));
				msg_write(format("samplerate: %d", FREQS[freq_index]));
				bool padding = ((data[2] & 0x02) > 0);
				int mode = ((data[3] & 0xc0) >> 6);
				msg_write(format("mode: %d", mode));

				if (error_correction)
					f->read_word();
				if (mode == 3) {
					// mono...
				} else {
					// 9+3+8+4*(12+9+8+4+1)
				}
				//f->SetPos(pos0 + size + 10, true);
				break;
			} else if ((data[0] == 'I') and (data[1] == 'D') and (data[2] == '3')) {
				msg_write("== ID3-Tags ==");
				int version = data[3];
				int v_min = f->read_byte();
				msg_write(format("v 2.%d.%d", version, v_min));
				int flags = f->read_byte();
				int size = read_mp3_28bit(f);
				//msg_write(size);
				int r = 0;
				while (r < size - 3) {
					if (version == 2) {
						data.resize(3);
						string key = data;
						//if (key[0] != 0)
						//	msg_write(key);
						data = f->read_buffer(3);
						unsigned int _size = data[2] + (data[1] << 8) + (data[0] << 16);
						//msg_write(_size);
						r += 6 + _size;
						if (_size < 1024) {
							data = f->read_buffer(_size);
						} else {
							f->seek(_size);
						}

					} else if ((version == 3) or (version == 4)) {
						string key = f->read_buffer(4);
						//if (key[0] != 0)
						//	msg_write(key);
						int _size;
						if (version == 4)
							_size = read_mp3_28bit(f);
						else
							_size = read_32bit_be(f);
						f->read_word(); // flags
						//msg_write(_size);
						r += 12 + _size;
						if ((_size < 1024) and (_size > 0)) {
							data = f->read_buffer(_size);
							string val = string(&data[1], _size-1);
							int type = data[0];
							if (key == "COMM")
								val = val.sub(3);
							if ((type == 1) or (type == 2))
								val = val.utf16_to_utf8();
							else if (type == 0)
								val = val.latin_to_utf8();
							val = val.replace(string("\0", 1), "");
							//msg_write(val);

							// will be added by wave loading...
							od->suggest_tag(tag_from_mp3(key), val);
						} else {
							f->seek(_size);
						}

					} else {
						od->error(format("unsupported ID3 version: v2.%d.%d", version, v_min));
						break;
					}
				}
				f->set_pos(pos0 + size + 10);
			} else {
				od->error("unknown header: " + data.hex());
				msg_write(data);
				break;
			}
		}

		f->set_pos(0);

		static mp3dec_t mp3d;
		mp3dec_init(&mp3d);

		data.clear();

		int channels = 2;

		od->info("mp3 decoder: https://github.com/lieff/minimp3");

		//msg_write("start");
		int sample_offset = 0;
		int bytes_offset = 0;
		while (true) {
			if (data.num < 4096*4) {
				bytes temp = f->read_buffer(4096*5 - data.num);
				//msg_write(format("R  %d", size));
				data += temp;
				od->set((float)f->get_pos() / (float)f->get_size());
			}

			mp3dec_frame_info_t info;
			float pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
			int samples = mp3dec_decode_frame(&mp3d, &data[0], data.num, pcm, &info);
			//msg_write(format("D  %d  %d", samples, info.frame_bytes));
			if ((samples == 0) and (info.frame_bytes == 0))
				break;
			channels = info.channels;
			import_data(od->layer, &pcm[0], channels, SampleFormat::SAMPLE_FORMAT_32_FLOAT, samples, sample_offset);
			sample_offset += samples;
			bytes_offset += info.frame_bytes;
			data = bytes(&data[info.frame_bytes], data.num - info.frame_bytes);
		}


		/*if (system("which avconv") == 0) {
			string tmp = "/tmp/tsunami_mp3_out.wav";
			system(format("yes | avconv -i \"%s\" \"%s\"", od->filename, tmp).c_str());
			od->storage->load_track(od->layer, tmp, od->offset);
			od->storage->current_directory = od->filename.parent();
			file_delete(tmp);
		} else if (system("which ffmpeg") == 0) {
			string tmp = "/tmp/tsunami_mp3_out.wav";
			system(format("yes | ffmpeg -i \"%s\" \"%s\"", od->filename, tmp).c_str());
			od->storage->load_track(od->layer, tmp, od->offset);
			od->storage->current_directory = od->filename.parent();
			file_delete(tmp);
		} else
			od->error("need external program 'avconv' to decode");*/


	} catch(Exception &e) {
		od->error(e.message());
	}

	if (f)
		FileClose(f);
}

