/*
 * FormatMp3.cpp
 *
 *  Created on: 18.09.2014
 *      Author: michi
 */

#include "FormatMp3.h"
#include "../../Tsunami.h"
#include "../../View/Helper/Progress.h"
#include "../../Stuff/Log.h"
#include "../Storage.h"
#include "../../lib/math/math.h"

FormatMp3::FormatMp3() :
	Format("Mp3", "mp3", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ)
{
}

FormatMp3::~FormatMp3()
{
}

// (-_-) 4*7bit big endian
static int read_mp3_28bit(File *f)
{
	unsigned char d[4];
	f->ReadBuffer(d, 4);

	return (d[3] & 0x7f) | ((d[2] & 0x7f) << 7) | ((d[1] & 0x7f) << 14) | ((d[0] & 0x7f) << 21);
}

static int read_32bit_be(File *f)
{
	unsigned char d[4];
	f->ReadBuffer(d, 4);

	return d[3] | (d[2] << 8) | (d[1] << 16) | (d[0] << 24);
}

void FormatMp3::saveBuffer(AudioFile *a, BufferBox *b, const string &filename){}

void FormatMp3::loadTrack(Track *t, const string & filename, int offset, int level)
{
	msg_db_f("load_mp3_file", 1);
	tsunami->progress->set(_("lade mp3"), 0);

	unsigned char *data = new unsigned char[4096];
	File *f = FileOpen(filename);

	try{

		if (!f)
			throw string("can't open file");
		f->SetBinaryMode(true);


		while(true){
			int pos0 = f->GetPos();
			f->ReadBuffer(data, 4);
			if ((data[0] == 0xff) && ((data[1] & 0xfe) == 0xfa)){
				msg_write("== mp3-header ==");
				int BIT_RATES[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
				int FREQS[] = {44100, 48000, 32000, 0};
				bool error_correction = ((data[1] & 0x01) > 0);
				msg_write(error_correction);
				int bit_rate_index = ((data[2] & 0xf0) >> 4);
				int freq_index = ((data[2] & 0x06) >> 2);
				int bit_rate = BIT_RATES[bit_rate_index];
				t->root->sample_rate = FREQS[freq_index];
				msg_write(bit_rate);
				msg_write(t->root->sample_rate);
				bool padding = ((data[2] & 0x02) > 0);
				int mode = ((data[3] & 0xc0) >> 6);
				msg_write(mode);

				if (error_correction)
					f->ReadBuffer(data, 2);
				if (mode == 3){
					// mono...
				}else{
					// 9+3+8+4*(12+9+8+4+1)
				}
				//f->SetPos(pos0 + size + 10, true);
				break;
			}else if ((data[0] == 'I') && (data[1] == 'D') && (data[2] == '3')){
				msg_write("== ID3-Tags ==");
				int version = data[3];
				int v_min = f->ReadByte();
				msg_write(format("v 2.%d.%d", version, v_min));
				int flags = f->ReadByte();
				int size = read_mp3_28bit(f);
				//msg_write(size);
				int r = 0;
				while (r < size - 3){
					if (version == 2){
						f->ReadBuffer(data, 3);
						string key = string(data, 3);
						//if (key[0] != 0)
						//	msg_write(key);
						f->ReadBuffer(data, 3);
						unsigned int _size = data[2] + (data[1] << 8) + (data[0] << 16);
						//msg_write(_size);
						r += 6 + _size;
						if (_size < 1024){
							f->ReadBuffer(data, _size);
						}else{
							f->SetPos(_size, false);
						}

					}else if ((version == 3) || (version == 4)){
						f->ReadBuffer(data, 4);
						string key = string(data, 4);
						//if (key[0] != 0)
						//	msg_write(key);
						int _size;
						if (version == 4)
							_size = read_mp3_28bit(f);
						else
							_size = read_32bit_be(f);
						f->ReadBuffer(data, 2); // flags
						//msg_write(_size);
						r += 12 + _size;
						if ((_size < 1024) && (_size > 0)){
							f->ReadBuffer(data, _size);
							string val = string(data+1, _size-1);
							int type = data[0];
							if (key == "COMM")
								val = val.substr(3, -1);
							if ((type == 1) || (type == 2))
								val = val.utf16_to_utf8();
							else if (type == 0)
								val = val.latin_to_utf8();
							val = val.replace(string("\0", 1), "");
							//msg_write(val);
							if (key == "TALB")
								t->root->addTag("album", val);
							else if (key == "TPE1")
								t->root->addTag("artist", val);
							else if (key == "TIT2")
								t->root->addTag("title", val);
							else if (key == "TRCK")
								t->root->addTag("track", i2s(val._int()));
							else if ((key == "TYER") || (key == "TDRC"))
								t->root->addTag("year", val);
							else if (key == "COMM")
								t->root->addTag("comment", val);
						}else{
							f->SetPos(_size, false);
						}

					}else{
						tsunami->log->error(format("mp3: unsupported ID3 version: v2.%d.%d", version, v_min));
						break;
					}
				}
				f->SetPos(pos0 + size + 10, true);
			}else{
				msg_write("unknown header:");
				msg_write(string(data, 4).hex());
				msg_write(string(data, 4));
				break;
			}
		}

		if (system("which avconv") == 0){
			string tmp = "/tmp/tsunami_mp3_out.wav";
			system(("avconv -i \"" + filename + "\" \"" + tmp + "\"").c_str());
			tsunami->storage->loadTrack(t, tmp, offset, level);
			tsunami->storage->current_directory = filename.dirname();
			file_delete(tmp);
		}else
			tsunami->log->error("mp3: need external program 'avconv' to decode");


	}catch(const string &s){
		tsunami->log->error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
}

void FormatMp3::saveAudio(AudioFile *a, const string & filename){}

void FormatMp3::loadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO);
	loadTrack(t, filename);
}

