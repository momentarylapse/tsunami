/*
 * FormatSoundFont2.cpp
 *
 *  Created on: 19.05.2015
 *      Author: michi
 */

#include "FormatSoundFont2.h"

FormatDescriptorSoundFont2::FormatDescriptorSoundFont2() :
	FormatDescriptor("SoundFont2", "sf2", FLAG_AUDIO | FLAG_TAGS | FLAG_SUBS | FLAG_READ)
{
}

FormatSoundFont2::FormatSoundFont2()
{
	sample_offset = 0;
	sample_count = 0;
}

void FormatSoundFont2::loadSong(StorageOperationData *_od)
{
	od = _od;
	song = od->song;

	File *f = NULL;

	sample_offset = -1;

	try{
		f = FileOpen(od->filename);
		read_chunk(f);
		if (sample_offset > 0)
			read_samples(f);
	}catch(Exception &e){
		od->error(e.message());
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
	f->read_buffer(temp, 4);
	string name = string(temp, 4);
	int l = f->read_int();
	int after_pos = f->get_pos() + l;

	msg_write("chunk: " + name + " (" + i2s(l) + ")");
	msg_right();


	if (name == "RIFF"){
		f->read_buffer(temp, 4);
		string aaa = string(temp, 4);
		if (aaa != "sfbk")
			throw Exception("'sfbk' expected in RIFF chunk");

		read_chunk(f);
		read_chunk(f);
		read_chunk(f);
	}else if (name == "LIST"){
		f->read_buffer(temp, 4);
		string aaa = string(temp, 4);
		msg_write("LIST: " + aaa);
		while (f->get_pos() < after_pos - 3){
			read_chunk(f);
		}
	}else if (name == "smpl"){
		sample_offset = f->get_pos();
		sample_count = l / 2;
	}else if (name == "shdr"){
		while (f->get_pos() < after_pos - 3){
			sfSample s;
			read_sample_header(f, s);
			if (s.name != "EOS")
				samples.add(s);
		}
	}

	f->set_pos(after_pos);


	msg_left();
}

void FormatSoundFont2::read_sample_header(File *f, FormatSoundFont2::sfSample &s)
{
	char temp[21];
	f->read_buffer(temp, 20);
	s.name = temp;
	s.start = f->read_int();
	s.end = f->read_int();
	s.start_loop = f->read_int();
	s.end_loop = f->read_int();
	s.sample_rate = f->read_int();
	s.original_key = f->read_byte();
	s.correction = f->read_char();
	s.sample_link = f->read_word();
	s.sample_type = f->read_word();
}

void FormatSoundFont2::read_samples(File *f)
{
	int samples_all = 0;
	int samples_read = 0;
	for (sfSample &s : samples)
		samples_all += s.end - s.start;
	for (sfSample &s : samples){
		//s.print();
		if ((s.sample_type & 0x8000) != 0){
			msg_write("rom");
			continue;
		}
		if ((s.start < 0) or (s.start >= sample_count))
			throw Exception(format("invalid sample start: %d   [0, %d)", s.start, sample_count));
		if ((s.end < 0) or (s.end >= sample_count))
			throw Exception(format("invalid sample end: %d   [0, %d)", s.end, sample_count));

		f->set_pos(sample_offset + s.start*2);
		AudioBuffer buf;
		int num_samples = s.end - s.start;
		if (num_samples < 0)
			throw Exception(format("negative sample size: %d - %d", s.start, s.end));
		char *data = new char[num_samples*2];
		f->read_buffer(data, num_samples*2);
		buf.resize(num_samples);
		buf.import(data, 1, SAMPLE_FORMAT_16, num_samples);// / 2);
		Sample *sample = song->addSample(s.name, buf);
		delete data;

		sample->tags.add(Tag("pitch", i2s(s.original_key)));
		sample->tags.add(Tag("start-loop", i2s(s.start_loop)));
		sample->tags.add(Tag("end-loop", i2s(s.end_loop)));
		sample->tags.add(Tag("correction", i2s(s.correction)));

		samples_read += num_samples;
		od->set(float(samples_read) / (float)samples_all);
	}
}
