/*
 * FormatMp3.cpp
 *
 *  Created on: 18.09.2014
 *      Author: michi
 */

#include "FormatMp3.h"
#include "../Storage.h"
#include "../../lib/math/math.h"

FormatDescriptorMp3::FormatDescriptorMp3() :
	FormatDescriptor("Mp3", "mp3", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ)
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

static string tag_from_mp3(const string &key)
{
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

void FormatMp3::loadTrack(StorageOperationData *od)
{
	msg_db_f("load_mp3_file", 1);
	Track *t = od->track;

	unsigned char *data = new unsigned char[4096];
	File *f = FileOpen(od->filename);

	try{

		if (!f)
			throw string("can not open file");
		f->SetBinaryMode(true);


		while(true){
			int pos0 = f->GetPos();
			f->ReadBuffer(data, 4);
			if ((data[0] == 0xff) and ((data[1] & 0xfe) == 0xfa)){
				msg_write("== mp3-header ==");
				int BIT_RATES[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
				int FREQS[] = {44100, 48000, 32000, 0};
				bool error_correction = ((data[1] & 0x01) > 0);
				msg_write(error_correction);
				int bit_rate_index = ((data[2] & 0xf0) >> 4);
				int freq_index = ((data[2] & 0x06) >> 2);
				int bit_rate = BIT_RATES[bit_rate_index];
				t->song->sample_rate = FREQS[freq_index];
				msg_write(bit_rate);
				msg_write(t->song->sample_rate);
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
			}else if ((data[0] == 'I') and (data[1] == 'D') and (data[2] == '3')){
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

					}else if ((version == 3) or (version == 4)){
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
						if ((_size < 1024) and (_size > 0)){
							f->ReadBuffer(data, _size);
							string val = string(data+1, _size-1);
							int type = data[0];
							if (key == "COMM")
								val = val.substr(3, -1);
							if ((type == 1) or (type == 2))
								val = val.utf16_to_utf8();
							else if (type == 0)
								val = val.latin_to_utf8();
							val = val.replace(string("\0", 1), "");
							//msg_write(val);

							// will be added by wave loading...
//							t->song->addTag(tag_from_mp3(key), val);
						}else{
							f->SetPos(_size, false);
						}

					}else{
						od->error(format("unsupported ID3 version: v2.%d.%d", version, v_min));
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
			system(("avconv -i \"" + od->filename + "\" \"" + tmp + "\"").c_str());
			od->storage->loadTrack(t, tmp, od->offset, od->level);
			od->storage->current_directory = od->filename.dirname();
			file_delete(tmp);
		}else
			od->error("need external program 'avconv' to decode");


	}catch(const string &s){
		od->error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
}

