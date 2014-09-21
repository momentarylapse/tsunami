/*
 * FormatMp3.cpp
 *
 *  Created on: 18.09.2014
 *      Author: michi
 */

#include "FormatMp3.h"
#include "../Tsunami.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"
#include "Storage.h"
#include "../lib/math/math.h"

FormatMp3::FormatMp3() :
	Format("Mp3", "mp3", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ)
{
}

FormatMp3::~FormatMp3()
{
}

// (-_-) 4*7bit big endian
static int read_mp3_28bit(CFile *f)
{
	unsigned char d[4];
	f->ReadBuffer(d, 4);

	return (d[3] & 0x7f) | ((d[2] & 0x7f) << 7) | ((d[1] & 0x7f) << 14) | ((d[0] & 0x7f) << 21);
}

static int read_32bit_be(CFile *f)
{
	unsigned char d[4];
	f->ReadBuffer(d, 4);

	return d[3] | (d[2] << 8) | (d[1] << 16) | (d[0] << 24);
}

string utf8_char(unsigned int code)
{
	char r[6] = "";
	if ((code & 0xffffff80) == 0){ // 7bit
		return string((char*)&code, 1);
	}else if ((code & 0xfffff800) == 0){ // 11bit
		r[1] = (code & 0x003f) | 0x80;        // 00-05
		r[0] = ((code & 0x07c0) >> 6) | 0xc0; // 06-10
		return string(r, 2);
	}else if ((code & 0xffff0000) == 0){ // 16bit
		r[2] = (code & 0x003f) | 0x80;         // 00-05
		r[1] = ((code & 0x0fc0) >> 6) | 0x80;  // 06-11
		r[0] = ((code & 0xf000) >> 12) | 0xe0; // 12-15
		return string(r, 3);
	}else if ((code & 0xffe00000) == 0){ // 21bit
		r[3] = (code & 0x0000003f) | 0x80;         // 00-05
		r[2] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[1] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[0] = ((code & 0x001c0000) >> 18) | 0xf0; // 18-20
		return string(r, 4);
	}else if ((code & 0xffe00000) == 0){ // 26bit
		r[4] = (code & 0x0000003f) | 0x80;         // 00-05
		r[3] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[2] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[1] = ((code & 0x00fc0000) >> 18) | 0x80; // 18-23
		r[1] = ((code & 0x03000000) >> 24) | 0xf4; // 24-25
		return string(r, 5);
	}else{ // 31bit
		r[5] = (code & 0x0000003f) | 0x80;         // 00-05
		r[4] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[3] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[2] = ((code & 0x00fc0000) >> 18) | 0x80; // 18-23
		r[1] = ((code & 0x3f000000) >> 24) | 0x80; // 24-29
		r[0] = ((code & 0x40000000) >> 30) | 0xfc; // 30
		return string(r, 6);
	}
}

static string utf16_to_utf8(const string &s)
{
	string r;
	bool big_endian = false;
	unsigned int last = 0;
	for (int i=0; i<s.num-1; i+=2){
		if (((unsigned char)s[i] == 0xff) && ((unsigned char)s[i+1] == 0xfe)){
			big_endian = false;
			continue;
		}else if (((unsigned char)s[i] == 0xfe) && ((unsigned char)s[i+1] == 0xff)){
			big_endian = true;
			continue;
		}
		unsigned int code = (unsigned char)s[i] | ((unsigned char)s[i+1] << 8);
		if (big_endian)
			code = (unsigned char)s[i+1] | ((unsigned char)s[i] << 8);
		//msg_write(string((char*)&code, 2).hex());

		if ((code < 0xd800) || (code > 0xdbff))
			r += utf8_char(code);
		else if ((last >= 0xdc00) && (last <= 0xdfff))
			r += utf8_char(0x010000 | ((code - 0xd800) << 12) | (last - 0xdc00));
		last = code;
	}
	return r;
}

static string latin_to_utf8(const string &s)
{
	string r;
	for (int i=0; i<s.num; i++)
		r += utf8_char(s[i]);
	return r;
}

void FormatMp3::SaveBuffer(AudioFile *a, BufferBox *b, const string &filename){}

void FormatMp3::LoadTrack(Track *t, const string & filename, int offset, int level)
{
	msg_db_f("load_mp3_file", 1);
	tsunami->progress->Set(_("lade mp3"), 0);

	char *data = new char[4096];
	CFile *f = FileOpen(filename);

	try{

		if (!f)
			throw string("can't open file");
		f->SetBinaryMode(true);


		while(true){
			int pos0 = f->GetPos();
			f->ReadBuffer(data, 4);
			if (((unsigned char)data[0] == 0xff) && ((unsigned char)(data[1] & 0xfe) == 0xfa)){
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
				while (r < size){
					if (version == 2){
						f->ReadBuffer(data, 3);
						msg_write(string(data, 3));
						f->ReadBuffer(data, 3);
						int _size = data[3] + (data[4] << 8) + (data[5] << 16);
						msg_write(_size);
						r += 6 + _size;
						f->ReadBuffer(data, _size);

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
								val = utf16_to_utf8(val);
							else if (type == 0)
								val = latin_to_utf8(val);
							val = val.replace(string("\0", 1), "");
							//msg_write(val);
							if (key == "TALB")
								t->root->AddTag("album", val);
							else if (key == "TPE1")
								t->root->AddTag("artist", val);
							else if (key == "TIT2")
								t->root->AddTag("title", val);
							else if (key == "TRCK")
								t->root->AddTag("track", i2s(val._int()));
							else if ((key == "TYER") || (key == "TDRC"))
								t->root->AddTag("year", val);
							else if (key == "COMM")
								t->root->AddTag("comment", val);
						}else{
							for (int i=0; i<_size; i++)
								f->ReadByte();
						}

					}else{
						tsunami->log->Error(format("mp3: unsupported ID3 version: v2.%d.%d", version, v_min));
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
			tsunami->storage->LoadTrack(t, tmp, offset, level);
			file_delete(tmp);
		}else
			tsunami->log->Error("mp3: need external program 'avconv' to decode");


	}catch(const string &s){
		tsunami->log->Error(s);
	}

	delete[](data);

	if (f)
		FileClose(f);
}

void FormatMp3::SaveAudio(AudioFile *a, const string & filename){}

void FormatMp3::LoadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->AddTrack(Track::TYPE_AUDIO);
	LoadTrack(t, filename);
}

