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
}

FormatSoundFont2::~FormatSoundFont2()
{
}

void FormatSoundFont2::loadTrack(Track* t, const string& filename, int offset, int level)
{
}

void FormatSoundFont2::saveBuffer(AudioFile* a, BufferBox* b, const string& filename)
{
}

void FormatSoundFont2::loadAudio(AudioFile* a, const string &filename)
{
	audio = a;

	CFile *f = FileOpen(filename);
	f->SetBinaryMode(true);

	try{
		tsunami->progress->set(_("importiere sf2"), 0);
		read_chunk(f);
		read_samples(f);
	}catch(string &s){
		tsunami->log->error(s);
	}


	delete(f);
}

void FormatSoundFont2::sfSample::print()
{
	msg_write("----sample");
	msg_write(string(name, 20));
	msg_write(start);
	msg_write(end);
	msg_write(start_loop);
	msg_write(end_loop);
	msg_write(sample_rate);
}

void FormatSoundFont2::read_chunk(CFile *f)
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
			throw "'sfbk' expected in RIFF chunk";

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
	}else if (name == "shdr"){
		while (f->GetPos() < after_pos - 3){
			sfSample s;
			f->ReadBuffer(&s, 46);//sizeof(s));
			if (strcmp(s.name, "EOS") != 0)
				samples.add(s);
		}
	}

	f->SetPos(after_pos, true);


	msg_left();
}

void FormatSoundFont2::read_samples(CFile *f)
{
	int samples_all = 0;
	int samples_read = 0;
	foreach(sfSample &s, samples)
		samples_all += s.end - s.start;
	foreach(sfSample &s, samples){
		//s.print();
		f->SetPos(sample_offset + s.start*2, true);
		BufferBox buf;
		int num_samples = s.end - s.start;
		if (num_samples < 0)
			throw "negative sample size";
		char *data = new char[num_samples*2];
		f->ReadBuffer(data, num_samples*2);
		buf.resize(num_samples);
		buf.import(data, 1, SAMPLE_FORMAT_16, num_samples);// / 2);
		audio->addSample(s.name, buf);
		delete data;

		samples_read += num_samples;
		tsunami->progress->set(float(samples_read) / (float)samples_all);
	}
}

void FormatSoundFont2::saveAudio(AudioFile* a, const string &filename)
{
}
