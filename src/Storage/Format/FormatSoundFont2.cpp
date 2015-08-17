/*
 * FormatSoundFont2.cpp
 *
 *  Created on: 19.05.2015
 *      Author: michi
 */

#include "FormatSoundFont2.h"
#include "../../Tsunami.h"
#include "../../Stuff/Log.h"
#include "../../View/Helper/Progress.h"

FormatSoundFont2::FormatSoundFont2() :
	Format("SoundFont2", "sf2", FLAG_AUDIO | FLAG_TAGS | FLAG_SUBS | FLAG_READ)
{
	song = NULL;
	sample_offset = 0;
	sample_count = 0;
	od = NULL;
}

FormatSoundFont2::~FormatSoundFont2()
{
}

void FormatSoundFont2::loadTrack(StorageOperationData *od)
{
}

void FormatSoundFont2::saveBuffer(StorageOperationData *od)
{
}

void FormatSoundFont2::loadSong(StorageOperationData *_od)
{
	od = _od;
	song = od->song;

	File *f = FileOpen(od->filename);
	f->SetBinaryMode(true);

	sample_offset = -1;

	try{
		read_chunk(f);
		if (sample_offset > 0)
			read_samples(f);
	}catch(string &s){
		tsunami->log->error(s);
	}


	delete(f);
}

void FormatSoundFont2::sfSample::print()
{
	msg_write("----sample");
	msg_write(name);
	msg_write(start);
	msg_write(end);
	msg_write(start_loop);
	msg_write(end_loop);
	msg_write(sample_rate);
	msg_write(sample_type);
}

void FormatSoundFont2::read_chunk(File *f)
{
	char temp[4];
	f->ReadBuffer(temp, 4);
	string name = string(temp, 4);
	int l = f->ReadInt();
	int after_pos = f->GetPos() + l;

	msg_write("chunk: " + name + " (" + i2s(l) + ")");
	msg_right();


	if (name == "RIFF"){
		f->ReadBuffer(temp, 4);
		string aaa = string(temp, 4);
		if (aaa != "sfbk")
			throw string("'sfbk' expected in RIFF chunk");

		read_chunk(f);
		read_chunk(f);
		read_chunk(f);
	}else if (name == "LIST"){
		f->ReadBuffer(temp, 4);
		string aaa = string(temp, 4);
		msg_write("LIST: " + aaa);
		while (f->GetPos() < after_pos - 3){
			read_chunk(f);
		}
	}else if (name == "smpl"){
		sample_offset = f->GetPos();
		sample_count = l / 2;
	}else if (name == "shdr"){
		while (f->GetPos() < after_pos - 3){
			sfSample s;
			read_sample_header(f, s);
			if (s.name != "EOS")
				samples.add(s);
		}
	}

	f->SetPos(after_pos, true);


	msg_left();
}

void FormatSoundFont2::read_sample_header(File *f, FormatSoundFont2::sfSample &s)
{
	char temp[21];
	f->ReadBuffer(temp, 20);
	s.name = temp;
	s.start = f->ReadInt();
	s.end = f->ReadInt();
	s.start_loop = f->ReadInt();
	s.end_loop = f->ReadInt();
	s.sample_rate = f->ReadInt();
	s.original_key = f->ReadByte();
	s.correction = f->ReadChar();
	s.sample_link = f->ReadWord();
	s.sample_type = f->ReadWord();
}

void FormatSoundFont2::read_samples(File *f)
{
	int samples_all = 0;
	int samples_read = 0;
	foreach(sfSample &s, samples)
		samples_all += s.end - s.start;
	foreach(sfSample &s, samples){
		//s.print();
		if ((s.sample_type & 0x8000) != 0){
			msg_write("rom");
			continue;
		}
		if ((s.start < 0) or (s.start >= sample_count))
			throw format("invalid sample start: %d   [0, %d)", s.start, sample_count);
		if ((s.end < 0) or (s.end >= sample_count))
			throw format("invalid sample end: %d   [0, %d)", s.end, sample_count);

		f->SetPos(sample_offset + s.start*2, true);
		BufferBox buf;
		int num_samples = s.end - s.start;
		if (num_samples < 0)
			throw format("negative sample size: %d - %d", s.start, s.end);
		char *data = new char[num_samples*2];
		f->ReadBuffer(data, num_samples*2);
		buf.resize(num_samples);
		buf.import(data, 1, SAMPLE_FORMAT_16, num_samples);// / 2);
		song->addSample(s.name, buf);
		delete data;

		samples_read += num_samples;
		od->progress->set(float(samples_read) / (float)samples_all);
	}
}

void FormatSoundFont2::saveSong(StorageOperationData *od)
{
}
