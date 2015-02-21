/*
 * FormatGp4.cpp
 *
 *  Created on: 01.10.2014
 *      Author: michi
 */

#include "FormatGp4.h"
#include "../Tsunami.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"

FormatGp4::FormatGp4() :
	Format("GuitarPro 4", "gp3,gp4,gp5", FLAG_MIDI | FLAG_READ | FLAG_MULTITRACK)
{
}

FormatGp4::~FormatGp4()
{
}

static string read_str1(CFile *f)
{
	int l = f->ReadByte();
	//msg_write(l);
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;
}

static string read_str1c(CFile *f, int size)
{
	int l = f->ReadByte();
	string s;
	s.resize(size);
	f->ReadBuffer(s.data, size);
	s.resize(l);
	return s;
}

static string read_str4(CFile *f)
{
	int l = f->ReadInt();
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;
}

static int read_int_be(CFile *f)
{
	int a = f->ReadByte();
	int b = f->ReadByte();
	int c = f->ReadByte();
	int d = f->ReadByte();
	return d | (c << 8) | (b << 16) | (a << 24);
}

static string read_str41(CFile *f)
{
	int l = f->ReadInt();
	/*msg_write(l);
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;*/
	return read_str1(f);
}

void FormatGp4::saveBuffer(AudioFile *a, BufferBox *b, const string &filename){}

void FormatGp4::loadTrack(Track *t, const string & filename, int offset, int level){}

void FormatGp4::saveAudio(AudioFile *a, const string & filename){}

void FormatGp4::loadAudio(AudioFile *_a, const string &filename)
{
	a = _a;
	f = FileOpen(filename);
	char *data = new char[1024];
	tracks.clear();
	measures.clear();

	try{

		if (!f)
			throw string("can't open file");
		f->SetBinaryMode(true);
		string s;
		s = read_str1c(f, 30);
		msg_write("version: "+s);
		version = -1;
		if (s[20] == '3')
			version = 300;
		if (s[20] == '4')
			version = 400;
		if (s[20] == '5'){
			version = 500;
			if (s[22] == '1')
				version = 510;
		}
		msg_write(version);

		read_info();

		if (version <= 400)
			int tripplet_feel = f->ReadByte();

		if (version >= 400)
			read_lyrics();
		if (version > 500)
			read_eq();
		if (version >= 500)
			read_page_setup();

		if (version >= 500)
			msg_write("tempo: " + read_str41(f));
		tempo = f->ReadInt();
		//msg_write(tempo);

		if (version > 500)
			f->ReadByte();

		if (version >= 400){
			f->ReadByte(); // key signature
			f->ReadBuffer(data, 3);
			f->ReadByte(); // octave
		}else{
			f->ReadInt(); // key
		}

		read_channels();

		if (version >= 500)
			f->SetPos(42, false);

		int num_measures = f->ReadInt();
		int num_tracks = f->ReadInt();
		msg_write(format("measures: %d   tracks: %d", num_measures, num_tracks));
		for (int i=0; i<num_measures; i++)
			read_measure_header();
		a->addTrack(Track::TYPE_TIME);
		for (int i=0; i<num_tracks; i++)
			read_track();

		if (version >= 500)
			f->ReadByte();

		int offset = 0;
		for (int i = 0; i < num_measures; i++){
			for (int j = 0; j < num_tracks; j++)
				read_measure(measures[i], tracks[j], offset);
			offset += a->sample_rate * 60.0f / (float)tempo * 4.0f * (float)measures[i].numerator / (float)measures[i].denominator;
			a->track[0]->addBars(-1, tempo, measures[i].numerator, 1);
		}

	}catch(const string &s){
		tsunami->log->error(s);
	}

	delete[](data);
	if (f)
		FileClose(f);
}

void FormatGp4::read_info()
{
	string s;
	s = read_str41(f);
	msg_write("title: "+s);
	if (s.num > 0)
		a->addTag("title", s);
	read_str41(f);
	s = read_str41(f);
	msg_write("artist: "+s);
	if (s.num > 0)
		a->addTag("artist", s);
	s = read_str41(f);
	msg_write("album: "+s);
	if (s.num > 0)
		a->addTag("album", s);
	if (version >= 500)
		msg_write("lyricist: "+read_str41(f));
	msg_write("author: "+read_str41(f));
	msg_write("copy: "+read_str41(f));
	msg_write("writer: "+read_str41(f));
	read_str41(f);
	int n = f->ReadInt();
	for (int i=0; i<n; i++)
		msg_write("comment: " + read_str41(f));
}

void FormatGp4::read_lyrics()
{
	int lyrics_track = f->ReadInt();
	//msg_write(lyrics_track);

	f->ReadInt();
	read_str4(f);
	for (int i=0; i<4; i++){
		f->ReadInt();
		read_str4(f);
	}
}

void FormatGp4::read_channels()
{
	for (int i = 0; i < 64; i++) {
		f->ReadInt(); // program
		f->ReadByte(); // volume
		f->ReadByte(); // balance
		f->ReadByte(); // chorus
		f->ReadByte(); // reverb
		f->ReadByte(); // phaser
		f->ReadByte(); // tremolo
		//if (i == 9) -> percussion
		f->SetPos(2, false);
	}
}

void FormatGp4::read_eq()
{
	f->ReadInt(); // master volume
	f->ReadInt();
	for (int i = 0; i < 11; i++)
		f->ReadByte(); // eq
}

void FormatGp4::read_page_setup()
{
	f->ReadInt();
	f->ReadInt();
	f->ReadInt();
	f->ReadInt();
	f->ReadInt();
	f->ReadInt();
	f->ReadInt();
	f->ReadWord();
	for (int i=0; i<10; i++)
		read_str41(f);
}

void FormatGp4::read_measure_header()
{
	GpMeasure m;
	if (measures.num > 0){
		m = measures.back();
	}else{
		m.numerator = 1;
		m.denominator = 1;
	}
	int flags = f->ReadByte();
	if ((flags & 0x01) != 0)
		m.numerator = f->ReadByte();
	if ((flags & 0x02) != 0)
		m.denominator = f->ReadByte();
	if ((flags & 0x08) != 0)
		f->ReadByte(); // repeat close
	if ((flags & 0x10) != 0)
		f->ReadByte(); // repeat alternative
	if ((flags & 0x20) != 0){
		m.marker = read_str41(f);
		f->ReadInt(); // color
	}
	if ((flags & 0x40) != 0){
		f->ReadByte();
		f->SetPos(1, false);
	}
	measures.add(m);
}

void FormatGp4::read_track()
{
	msg_db_f("track", 1);
	f->ReadByte();
	GpTrack tt;
	tt.t = a->addTrack(Track::TYPE_MIDI);
	tt.t->setName(read_str1c(f, 40));
	tt.stringCount = f->ReadInt();
	for (int i=0; i<7; i++){
		int tuning = f->ReadInt(); // tuning
		tt.tuning.add(tuning);
	}
	tracks.add(tt);
	f->ReadInt();
	read_channel();
	f->ReadInt();
	f->ReadInt(); // offset
	f->ReadInt(); // color
	if (version > 500){
		f->SetPos(49, false);
		read_str41(f);
		read_str41(f);
	}
	if (version == 500)
		f->SetPos(45, false);
}

void FormatGp4::read_channel()
{
	f->ReadInt();
	f->ReadInt();
}

void FormatGp4::read_measure(GpMeasure &m, GpTrack &t, int offset)
{
	msg_db_f("measure", 1);
	int beats = f->ReadInt();
	//msg_write(beats);
	if (beats > 1000)
		throw string("too many beats...");
	for (int i=0; i<beats; i++){
		int length = read_beat(t, m, offset);
		offset += length;
	}
}

int FormatGp4::read_beat(GpTrack &t, GpMeasure &m, int start)
{
	msg_db_f("beat", 1);
	int flags = f->ReadByte();
	if((flags & 0x40) != 0)
		f->ReadByte();

	int duration = read_duration(flags, m);
	//msg_write(duration);

	if ((flags & 0x02) != 0)
		read_chord();
	if ((flags & 0x04) != 0){
		//read_text(beat);
		msg_write("text: " + read_str41(f));
	}
	if ((flags & 0x08) != 0)
		read_beat_fx();
	if ((flags & 0x10) != 0)
		read_mix_change();
	int stringFlags = f->ReadByte();
	for (int i = 6; i >= 0; i--) {
		if ((stringFlags & (1 << i)) != 0 && (6 - i) < t.stringCount) {
			//TGString string = track.getString( (6 - i) + 1 ).clone(getFactory());
			read_note(t, (6 - i), start, duration);
			//TGNote note = readNote(string, track,effect.clone(getFactory()));
			//voice.addNote(note);
		}
	}
	return duration;
}

void FormatGp4::read_chord()
{
	msg_db_f("chord", 1);
	int type = f->ReadByte();
	if ((type & 0x01) == 0){
		string name = read_str41(f);
		int first_fret = f->ReadInt();
		if (first_fret != 0){
			for (int i=0; i<6; i++) {
				int fret = f->ReadInt();
				/*if(i < chord.countStrings()){
					chord.addFretValue(i,fret);
				}*/
			}
		}
	}else{
		f->SetPos(16, false);
		string name = read_str1c(f, 21);
		f->SetPos(4, false);
		int first_fret = f->ReadInt();
		for (int i=0; i<7; i++){
			int fret = f->ReadInt();
			/*if(i < chord.countStrings()){
				chord.addFretValue(i,fret);
			}*/
		}
		f->SetPos(32, false);
	}
}

void FormatGp4::read_note(GpTrack &t, int string_no, int start, int length)
{
	msg_db_f("note", 1);
	MidiNote n;
	n.range = Range(start, length);
	n.volume = 1;
	n.pitch = -1;
	int flags = f->ReadByte();
	if ((flags & 0x20) != 0) {
		int noteType = f->ReadByte();
	}
	if ((flags & 0x01) != 0)
		f->SetPos(2, false);
	if ((flags & 0x10) != 0)
		n.volume = 0.1f + 0.9f * (float)f->ReadByte() / 10.0f;
	if ((flags & 0x20) != 0) {
		int fret = f->ReadByte();
		int value = fret;
		if ((string_no >= 0) && (string_no < t.tuning.num))
			value = fret + t.tuning[string_no];
		n.pitch = value;
	}
	if ((flags & 0x80) != 0)
		f->SetPos(2, false);
	if ((flags & 0x08) != 0) {
		read_note_fx();
	}
	if (n.volume > 1)
		n.volume = 1;
	if (n.pitch >= 0)
		t.t->addMidiNote(n);
}

void FormatGp4::read_note_fx()
{
	msg_db_f("note fx", 1);
	int flags1 = f->ReadByte();
	int flags2 = 0;
	if (version >= 400)
		flags2 = f->ReadByte();
	if ((flags1 & 0x01) != 0) {
		// bend
		f->SetPos(5, false);
		int points = f->ReadInt();
		for (int i=0; i<points; i++){
			int position = f->ReadInt();
			int value = f->ReadInt();
			f->ReadByte();
		}
	}
	if ((flags1 & 0x10) != 0) {
		// grace
		int fret = f->ReadByte();
		int volume = f->ReadByte();
		int transition = f->ReadByte();
		int duration = f->ReadByte();
	}
	if ((flags2 & 0x04) != 0){
		// tremolo picking
		f->ReadByte();
	}
	if ((flags2 & 0x08) != 0)
		f->ReadByte();
	if ((flags2 & 0x10) != 0)
		int type = f->ReadByte(); // harmonic
	if ((flags2 & 0x20) != 0) {
		int fret = f->ReadByte();
		int period = f->ReadByte();
	}
}

int FormatGp4::read_duration(int flags, GpMeasure &m)
{
	msg_db_f("duration", 1);
	int v = (signed char)f->ReadByte();
	float value = 16.0f / (float)(int)(1 << (v + 4));
	bool dotted = ((flags & 0x01) != 0);
	if (dotted)
		value *= 1.5f;
	if ((flags & 0x20) != 0) {
		int divisionType = f->ReadInt();
		switch (divisionType) {
		case 3:
			//duration.getDivision().setEnters(3);
			//duration.getDivision().setTimes(2);
			break;
		case 5:
			//duration.getDivision().setEnters(5);
			//duration.getDivision().setTimes(4);
			break;
		case 6:
			//duration.getDivision().setEnters(6);
			//duration.getDivision().setTimes(4);
			break;
		case 7:
			//duration.getDivision().setEnters(7);
			//duration.getDivision().setTimes(4);
			break;
		case 9:
			//duration.getDivision().setEnters(9);
			//duration.getDivision().setTimes(8);
			break;
		case 10:
			//duration.getDivision().setEnters(10);
			//duration.getDivision().setTimes(8);
			break;
		case 11:
			//duration.getDivision().setEnters(11);
			//duration.getDivision().setTimes(8);
			break;
		case 12:
			//duration.getDivision().setEnters(12);
			//duration.getDivision().setTimes(8);
			break;
		}
	}
	return a->sample_rate * value * 60.0f / (float)tempo;
}

void FormatGp4::read_mix_change()
{
	msg_db_f("mix", 1);
	f->ReadByte();
	int volume = (signed char)f->ReadByte();
	int pan = (signed char)f->ReadByte();
	int chorus = (signed char)f->ReadByte();
	int reverb = (signed char)f->ReadByte();
	int phaser = (signed char)f->ReadByte();
	int tremolo = (signed char)f->ReadByte();
	int tempoValue = f->ReadInt();
	//msg_write(format("%d %d %d %d %d %d %d", volume, pan, chorus, reverb, phaser, tremolo, tempoValue));
	if (volume >= 0)
		f->ReadByte();
	if (pan >= 0)
		f->ReadByte();
	if (chorus >= 0)
		f->ReadByte();
	if (reverb >= 0)
		f->ReadByte();
	if (phaser >= 0)
		f->ReadByte();
	if (tremolo >= 0)
		f->ReadByte();
	if (tempoValue >= 0){
		tempo = tempoValue;
		f->ReadByte();
	}
	f->ReadByte();
}

void FormatGp4::read_beat_fx()
{
	msg_db_f("beat fx", 1);
	int flags1 = f->ReadByte();
	int flags2 = 0;
	if (version >= 400)
		flags2 = f->ReadByte();
	if ((flags1 & 0x20) != 0) {
		int effect = f->ReadByte();
		if (version < 400)
			f->ReadInt();
	}
	if ((flags2 & 0x04) != 0) {
		// tremolo
		f->SetPos(5, false);
		int points = f->ReadInt();
		//msg_write(points);
		for (int i=0; i<points; i++){
			int position = f->ReadInt();
			int value = f->ReadInt();
			f->ReadByte();
		}
	}
	if ((flags1 & 0x40) != 0) {
		int strokeDown = f->ReadByte();
		int strokeUp = f->ReadByte();
	}
	if ((flags2 & 0x02) != 0)
		f->ReadByte();
}
