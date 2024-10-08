/*
 * FormatSoundFont2.cpp
 *
 *  Created on: 19.05.2015
 *      Author: michi
 */

#include "FormatSoundFont2.h"
#include "../../lib/base/iter.h"
#include "../../lib/os/file.h"
#include "../../lib/os/formatter.h"
#include "../../lib/os/msg.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../Session.h"
#include <cmath>

namespace tsunami {

FormatDescriptorSoundFont2::FormatDescriptorSoundFont2() :
	FormatDescriptor("SoundFont2", "sf2", Flag::Audio | Flag::Tags | Flag::Samples | Flag::Read)
{
}

FormatSoundFont2::FormatSoundFont2() {
	sample_offset = 0;
	sample_count = 0;
	od = nullptr;
	song = nullptr;
}

Any ia2any(const Array<int> &ia) {
	Any a;
	for (int i: ia)
		a.add(i);
	return a;
}

float abs_timecents_to_sec(int ats) {
	return (float)pow(2.0, ats / 1200.0);
}

int sec_to_samples(Session *s, float t) {
	return int(s->sample_rate() * t);
}

void import_zones(Any &ai, const Array<FormatSoundFont2::sfZone> &zones, int zone_start, int zone_end, const Array<FormatSoundFont2::sfGenerator> &generators) {
	Any azs;
	for (auto&& [ii,z]: enumerate(zones)) {
		if (ii >= zone_start and ii < zone_end) {
			int start = 0;
			int end = 0;
			int startloop = 0;
			int endloop = 0;
			Any az;
			msg_write(format("  zone %d", ii));
			for (int jj=z.gen_start; jj<z.gen_end; jj++) {
				auto &g = generators[jj];
				msg_write("    gen " + g.str());
				if (g.op <= 4)
					msg_error("WAHHHH");
				if (g.op == 43) {
					az.dict_set("keys", ia2any({g.amount & 0xff, (g.amount >> 8) & 0xff}));
				} else if (g.op == 44) {
					az.dict_set("vel", ia2any({g.amount & 0xff, (g.amount >> 8) & 0xff}));
				} else if (g.op == 46) {
					az.dict_set("keys", ia2any({g.amount, g.amount}));
				} else if (g.op == 47) {
					az.dict_set("vel", ia2any({g.amount, g.amount}));
				} else if (g.op == 53) {
					az.dict_set("sample", g.amount);
				} else if (g.op == 58) {
					az.dict_set("root-key", g.amount);
				} else if (g.op == 41) {
					az.dict_set("instrument", g.amount);
				} else if (g.op == 2) {
					startloop += g.amount;
				} else if (g.op == 3) {
					endloop += g.amount;
				} else if (g.op == 45) {
					startloop += g.amount << 15;
				} else if (g.op == 50) {
					endloop += g.amount << 15;
				} else if (g.op == 0) {
					start += g.amount;
				} else if (g.op == 1) {
					end += g.amount;
				} else if (g.op == 4) {
					start += g.amount << 15;
				} else if (g.op == 12) {
					end += g.amount << 15;
				} else if (g.op == 34) {
					az.dict_set("attack", abs_timecents_to_sec((short)g.amount));
				} else if (g.op == 38) {
					az.dict_set("release", abs_timecents_to_sec((short)g.amount));
				}
			}
			if (start > 0)
				az.dict_set("start", start);
			if (end > 0)
				az.dict_set("end", end);
			if (startloop > 0)
				az.dict_set("start-loop", startloop);
			if (endloop > 0)
				az.dict_set("end-loop", endloop);
			if (!az.is_empty())
				azs.add(az);
		}
	}
	ai.dict_set("zones", azs);
}

void FormatSoundFont2::load_song(StorageOperationData *_od) {
	od = _od;
	song = od->song;

	os::fs::FileStream *f = nullptr;

	sample_offset = -1;

	song->add_track(SignalType::Audio);

	try {
		f = os::fs::open(od->filename, "rb");
		read_chunk(f);
		if (sample_offset > 0)
			read_samples(f);
	} catch(Exception &e) {
		od->error(e.message());
	}


	presets.back().zone_end = preset_zones.num;
	preset_zones.back().gen_end = preset_generators.num;
	Any aps;
	for (auto &p: presets) {
		msg_write(format("preset: %s", p.name));
		Any ap;
		ap.dict_set("name", p.name);
		import_zones(ap, preset_zones, p.zone_start, p.zone_end, preset_generators);
		aps.add(ap);
	}
	song->secret_data.dict_set("presets", aps);

	instruments.back().zone_end = instrument_zones.num;
	instrument_zones.back().gen_end = instrument_generators.num;
	Any ais;
	for (auto &i: instruments) {
		msg_write(format("instrument: %s", i.name));
		Any ai;
		ai.dict_set("name", i.name);
		import_zones(ai, instrument_zones, i.zone_start, i.zone_end, instrument_generators);
		ais.add(ai);
	}

	song->secret_data.dict_set("instruments", ais);
	//msg_write(song->secret_data.str());

	delete f;
}

void FormatSoundFont2::sfSample::print() {
	msg_write("----sample");
	msg_write(name);
	msg_write(start);
	msg_write(end);
	msg_write(start_loop);
	msg_write(end_loop);
	msg_write(sample_rate);
	msg_write(sample_type);
}

string read_str(Stream *f, int l) {
	string s = f->read(l);
	int p0 = s.find(string("\0", 1), 0);
	if (p0 >= 0)
		return s.head(p0);
	return s;
}

string FormatSoundFont2::sfGenerator::str() const {
	string s = "? " + i2s(op);
	if (op == 0)	s = "start addrs offset+";
	if (op == 1)	s = "end addrs offset+";
	if (op == 2)	s = "start loop addrs offset+";
	if (op == 3)	s = "end loop addrs offset+";
	if (op == 4)	s = "start addrs coarse offset+";
	if (op == 8)	s = "initial filter fc";
	if (op == 15)	s = "chorus send";
	if (op == 16)	s = "reverb send";
	if (op == 17)	s = "pan";
	if (op == 34)	s = "attack";
	if (op == 35)	s = "hold";
	if (op == 36)	s = "decay";
	if (op == 37)	s = "sustain";
	if (op == 38)	s = "release";
	if (op == 41)	s = "instrument";
	if (op == 43)	s = "key range";
	if (op == 44)	s = "vel range";
	if (op == 45)	s = "start loop addrs coarse offset+";
	if (op == 46)	s = "key";
	if (op == 47)	s = "vel";
	if (op == 50)	s = "end loop addrs coarse offset+";
	if (op == 53)	s = "sample";
	if (op == 54)	s = "sample modes";
	if (op == 58)	s = "root key";
	s += format("  %04x", amount);
	return s;
}

void FormatSoundFont2::read_chunk(Stream *f) {
	string name = read_str(f, 4).upper();
	int l = f->read_int();
	int after_pos = f->pos() + l;

	od->session->debug("sf2", format("chunk: %s (%d)", name, l));


	if (name == "RIFF") {
		string aaa = read_str(f, 4);
		if (aaa != "sfbk")
			throw Exception("'sfbk' expected in 'RIFF' chunk");

		read_chunk(f);
		read_chunk(f);
		read_chunk(f);
	} else if (name == "LIST") {
		string aaa = read_str(f, 4);
		od->session->debug("sf2", format("list type: %s", aaa));
		while (f->pos() < after_pos - 3) {
			read_chunk(f);
		}
	} else if (name == "SMPL") {
		sample_offset = f->pos();
		sample_count = l / 2;
	} else if (name == "SHDR") {
		while (f->pos() < after_pos - 3) {
			sfSample s;
			read_sample_header(f, s);
			if (s.name == "EOS")
				break;
			samples.add(s);
		}
	} else if (sa_contains({"INAM", "IENG", "IPRD", "ICOP", "ICRD", "ICMT", "ISFT", "ISNG"}, name)) {
		string t = read_str(f, l);
		if (name == "INAM")
			song->add_tag("title", t);
		else if (name == "IENG")
			song->add_tag("engineer", t);
		else if (name == "IPRD")
			song->add_tag("product", t);
		else if (name == "ICOP")
			song->add_tag("copyright", t);
		else if (name == "ICRD")
			song->add_tag("date", t);
		else if (name == "ICMT")
			song->add_tag("comment", t);
		else if (name == "ISNG")
			song->add_tag("engine", t);
		else if (name == "ISFT")
			song->add_tag("software", t);
	} else if (sa_contains({"IFIL", "PMOD", "IMOD"}, name)) {
		// ignore
	} else if (name == "PHDR") {
		while (f->pos() < after_pos - 3) {
			sfPresetHeader p;
			p.name = read_str(f, 20);
			p.preset = f->read_word();
			p.bank = f->read_word();
			p.zone_start = f->read_word();
			p.zone_end = -1;
			p.library = f->read_int();
			p.genre = f->read_int();
			p.morphology = f->read_int();
			if (p.name == "EOP")
				break;
			if (presets.num > 0)
				presets.back().zone_end = p.zone_start;
			presets.add(p);
		}
	} else if (name == "INST") {
		while (f->pos() < after_pos - 3) {
			sfInstrument i;
			i.name = read_str(f, 20);
			i.zone_start = f->read_word();
			i.zone_end = -1;
			if (i.name == "EOI")
				break;
			if (instruments.num > 0)
				instruments.back().zone_end = i.zone_start;
			instruments.add(i);
		}
	} else if (name == "PBAG") {
		while (f->pos() < after_pos - 3) {
			sfZone p;
			p.gen_start = f->read_word();
			p.gen_end = -1;
			p.mod_index = f->read_word();
			if (preset_zones.num > 0)
				preset_zones.back().gen_end = p.gen_start;
			preset_zones.add(p);
		}
	} else if (name == "IBAG") {
		while (f->pos() < after_pos - 3) {
			sfZone p;
			p.gen_start = f->read_word();
			p.gen_end = -1;
			p.mod_index = f->read_word();
			if (instrument_zones.num > 0)
				instrument_zones.back().gen_end = p.gen_start;
			instrument_zones.add(p);
		}
	} else if (name == "PGEN") {
		while (f->pos() < after_pos - 3) {
			sfGenerator g;
			g.op = f->read_word();
			g.amount = f->read_word();
			preset_generators.add(g);
		}
	} else if (name == "IGEN") {
		while (f->pos() < after_pos - 3) {
			sfGenerator g;
			g.op = f->read_word();
			g.amount = f->read_word();
			instrument_generators.add(g);
		}
	} else {
		bytes t = f->read(l);
		od->session->debug("sf2", t.hex());
	}

	f->set_pos(after_pos);
}

void FormatSoundFont2::read_sample_header(Stream *f, FormatSoundFont2::sfSample &s) {
	s.name = read_str(f, 20);
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

void FormatSoundFont2::read_samples(Stream *f) {
	int samples_all = 0;
	int samples_read = 0;
	for (auto &s : samples)
		samples_all += s.end - s.start;
	for (auto &s : samples) {
		//s.print();
		if ((s.sample_type & 0x8000) != 0) {
			od->session->debug("sf2", "rom");
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
		f->read(data, num_samples*2);
		buf.resize(num_samples);
		buf.import(data, 1, SampleFormat::Int16, num_samples);// / 2);
		Sample *sample = song->create_sample_audio(s.name, buf);
		delete[] data;

		sample->tags.add({"pitch", i2s(s.original_key)});
		sample->tags.add({"start-loop", i2s(s.start_loop - s.start)});
		sample->tags.add({"end-loop", i2s(s.end_loop - s.start)});
		sample->tags.add({"correction", i2s(s.correction)});

		samples_read += num_samples;
		od->set(float(samples_read) / (float)samples_all);
	}
}

}
