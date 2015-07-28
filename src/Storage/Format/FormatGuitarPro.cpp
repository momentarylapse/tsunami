/*
 * FormatGuitarPro.cpp
 *
 *  Created on: 01.10.2014
 *      Author: michi
 */

#include "../../Tsunami.h"
#include "../../View/Helper/Progress.h"
#include "../../Stuff/Log.h"
#include "FormatGuitarPro.h"

const int STD_TUNING[6] = {40, 45, 50, 55, 59, 64};

FormatGuitarPro::FormatGuitarPro() :
	Format("GuitarPro", "gp3,gp4,gp5", FLAG_MIDI | FLAG_READ | FLAG_WRITE | FLAG_MULTITRACK)
{
}

FormatGuitarPro::~FormatGuitarPro()
{
}


static void write_str1(File *f, const string &s)
{
	f->WriteByte(s.num);
	f->WriteBuffer(s.data, s.num);
}

static void write_str1c(File *f, const string &s, int size)
{
	f->WriteByte(s.num);
	string t = s;
	t.resize(size);
	f->WriteBuffer(t.data, size);
}

static void write_str4(File *f, const string &s)
{
	f->WriteInt(s.num);
	f->WriteBuffer(s.data, s.num);
}

static void write_str41(File *f, const string &s)
{
	f->WriteInt(s.num + 1);
	write_str1(f, s);
}

static string read_str1(File *f)
{
	int l = f->ReadByte();
	//msg_write(l);
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;
}

static string read_str1c(File *f, int size)
{
	int l = f->ReadByte();
	string s;
	s.resize(size);
	f->ReadBuffer(s.data, size);
	s.resize(l);
	return s;
}

static string read_str4(File *f)
{
	int l = f->ReadInt();
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;
}

static string read_str41(File *f)
{
	int l = f->ReadInt();
	/*msg_write(l);
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;*/
	return read_str1(f);
}

void FormatGuitarPro::saveBuffer(StorageOperationData *od){}

void FormatGuitarPro::loadTrack(StorageOperationData *od){}

void FormatGuitarPro::saveAudio(StorageOperationData *od)
{
	a = od->audio;
	char data[16];

	f = FileCreate(od->filename);
	f->SetBinaryMode(true);
	string ext = od->filename.extension();
	if (ext == "gp3")
		version = 300;
	else if (ext == "gp4")
		version = 400;
	else
		version = 500;

	if (version == 300)
		write_str1c(f, "FICHIER GUITAR PRO v3.00", 30);
	else if (version == 400)
		write_str1c(f, "FICHIER GUITAR PRO v4.00", 30);
	else //if (version == 500)
		write_str1c(f, "FICHIER GUITAR PRO v5.00", 30);

	Track *tt = a->getTimeTrack();
	Array<Track*> tracks;
	foreach(Track *t, a->tracks)
		if (t != tt)
			tracks.add(t);
	Array<Bar> bars = tt->bars.getBars(Range(-1000000000, 2000000000));
	tempo = 60.0f * (float)bars[0].num_beats / (float)bars[0].range.num * (float)a->sample_rate;


	write_info();

	if (version < 500)
		f->WriteByte(0); // tripplet feel

	if (version >= 400)
		write_lyrics();
	if (version > 500)
		write_eq();
	if (version >= 500)
		write_page_setup();

	if (version >= 500)
		write_str41(f, "");
	f->WriteInt(tempo);

	if (version > 500)
		f->WriteByte(0); // ???

	if (version >= 400){
		f->WriteByte(0); // key signature
		f->WriteBuffer(data, 3);
		f->WriteByte(0); // octave
	}else{
		f->WriteInt(0); // key
	}

	write_channels();

	if (version >= 500)
		f->SetPos(42, false);

	f->WriteInt(bars.num);
	f->WriteInt(tracks.num);
	foreach(Bar &b, bars)
		write_measure_header(b);
	foreach(Track *t, tracks)
		write_track(t);

	if (version >= 500)
		f->WriteByte(0);

	foreach(Bar &b, bars){
		foreach(Track *t, tracks)
			write_measure(t, b);
	}
	delete(f);
}

void FormatGuitarPro::loadAudio(StorageOperationData *od)
{
	a = od->audio;
	f = FileOpen(od->filename);
	char data[16];
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

		if (version < 500)
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
			a->tracks[0]->addBar(-1, tempo, measures[i].numerator);
		}

	}catch(const string &s){
		tsunami->log->error(s);
	}

	if (f)
		FileClose(f);
}

void FormatGuitarPro::read_info()
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

void FormatGuitarPro::write_info()
{
	write_str41(f, a->getTag("title"));
	write_str41(f, "");
	write_str41(f, a->getTag("artist"));
	write_str41(f, a->getTag("album"));
	if (version >= 500)
		write_str41(f, a->getTag("lyricist"));
	write_str41(f, a->getTag("author"));
	write_str41(f, a->getTag("copy"));
	write_str41(f, a->getTag("writer"));
	write_str41(f, "");
	f->WriteInt(0); // #comments
}

void FormatGuitarPro::read_lyrics()
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

void FormatGuitarPro::write_lyrics()
{
	f->WriteInt(0); // lyrics track
	//msg_write(lyrics_track);

	f->WriteInt(0);
	write_str4(f, "");
	for (int i=0; i<4; i++){
		f->WriteInt(0);
		write_str4(f, "");
	}
}

void FormatGuitarPro::read_channels()
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

void FormatGuitarPro::write_channels()
{
	for (int i = 0; i < 64; i++) {
		f->WriteInt(0); // program
		f->WriteByte(100); // volume
		f->WriteByte(0); // balance
		f->WriteByte(0); // chorus
		f->WriteByte(0); // reverb
		f->WriteByte(0); // phaser
		f->WriteByte(0); // tremolo
		//if (i == 9) -> percussion
		f->SetPos(2, false);
	}
}

void FormatGuitarPro::read_eq()
{
	f->ReadInt(); // master volume
	f->ReadInt();
	for (int i = 0; i < 11; i++)
		f->ReadByte(); // eq
}

void FormatGuitarPro::write_eq()
{
	f->WriteInt(100); // master volume
	f->WriteInt(0);
	for (int i = 0; i < 11; i++)
		f->WriteByte(100); // eq
}

void FormatGuitarPro::read_page_setup()
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

void FormatGuitarPro::write_page_setup()
{
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteWord(0);
	for (int i=0; i<10; i++)
		write_str41(f, "");
}

void FormatGuitarPro::read_measure_header()
{
	msg_db_f("bar", 1);
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
	if (((flags & 0x10) != 0) and (version < 500))
		f->ReadByte(); // repeat alternative
	if ((flags & 0x20) != 0){
		m.marker = read_str41(f);
		f->ReadInt(); // color
	}
	if (((flags & 0x10) != 0) and (version >= 500))
		f->ReadByte(); // repeat alternative
	if ((flags & 0x40) != 0){
		f->ReadByte(); // key
		f->ReadByte();
	}
	if (version >= 500){
		if ((flags & 0x03) != 0)
			f->ReadInt(); // beam 8 notes
		if ((flags & 0x10) == 0)
			f->ReadByte(); // repeat alternative
		f->ReadByte(); // triplet feel
		f->ReadByte();
	}
	measures.add(m);
}

void FormatGuitarPro::write_measure_header(Bar &b)
{
	msg_db_f("bar", 1);
	f->WriteByte(0x03);
	f->WriteByte(b.num_beats);
	f->WriteByte(4);
	if (version >= 500){
		//if ((flags & 0x03) != 0)
			f->WriteInt(0); // beam 8 notes
		//if ((flags & 0x10) == 0)
			f->WriteByte(0); // repeat alternative
		f->WriteByte(0); // triplet feel
		f->WriteByte(0);
	}
}

void FormatGuitarPro::read_track()
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
	msg_write("tuning: " + ia2s(tt.tuning));
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

void FormatGuitarPro::write_track(Track *t)
{
	msg_db_f("track", 1);
	f->WriteByte(0);
	GpTrack tt;
	write_str1c(f, t->name, 40);
	// tuning
	f->WriteInt(6); // string count
	for (int i=5; i>=0; i--)
		f->WriteInt(STD_TUNING[i]);
	f->WriteInt(0);

	f->WriteInt(1);
	write_channel();
	f->WriteInt(24);
	f->WriteInt(0); // offset
	f->WriteInt(0); // color
	if (version > 500){
		f->SetPos(49, false);
		write_str41(f, "");
		write_str41(f, "");
	}
	if (version == 500)
		f->SetPos(45, false);
}

void FormatGuitarPro::read_channel()
{
	f->ReadInt();
	f->ReadInt();
}

void FormatGuitarPro::write_channel()
{
	f->WriteInt(1);
	f->WriteInt(2);
}

void FormatGuitarPro::read_measure(GpMeasure &m, GpTrack &t, int offset)
{
	msg_db_f("measure", 1);
	//msg_write(format("%x", f->GetPos()));

	int num_voices = 1;
	if (version >= 500)
		num_voices = 2;

	for (int v=0; v<num_voices; v++){

		int num_beats = f->ReadInt();
		//msg_write(num_beats);
		if (num_beats > 1000)
			throw string("too many beats... " + i2s(num_beats));
		for (int i=0; i<num_beats; i++){
			int length = read_beat(t, m, offset);
			offset += length;
		}
	}

	if (version >= 500)
		f->ReadByte();
}

struct GuitarNote
{
	int offset, length;
	Array<int> pitch;
	Array<int> string;
	void detune()
	{
		for (int i=0; i<pitch.num; i++)
			for (int j=i+1; j<pitch.num; j++)
				if (pitch[i] >= pitch[j]){
					int t = pitch[i];
					pitch[i] = pitch[j];
					pitch[j] = t;
				}

		string.resize(pitch.num);
		int prev = -1;
		for (int n=0; n<pitch.num; n++){
			for (int i=5; i>prev; i--){
				if (pitch[n] >= STD_TUNING[i]){
					string[n] = i;
					pitch[n] -= STD_TUNING[i];
					break;
				}
			}
			prev = string[n];
		}
	}
};

Array<GuitarNote> create_guitar_notes(Track *t, Bar &b)
{
	// samples per 16th
	float spu = (float)b.range.num / (float)b.num_beats / 4.0f;

	Array<MidiNote> notes = t->midi.getNotes(b.range);
	Array<GuitarNote> gnotes;

	foreach(MidiNote &n, notes){
		GuitarNote gn;
		gn.offset = int((float)(n.range.offset - b.range.offset) / spu + 0.5f);
		gn.length = int((float)n.range.num / spu + 0.5f);
		gn.pitch.add(n.pitch);
		if (gn.length == 0)
			continue;
		if (gn.offset < b.num_beats * 4)
			gnotes.add(gn);
	}

	// merge
	for (int i=0; i<gnotes.num; i++)
		for (int j=i+1; j<gnotes.num; j++)
			if (gnotes[i].offset == gnotes[j].offset){
				gnotes[i].pitch.append(gnotes[j].pitch);
				gnotes.erase(j);
				j --;
			}

	// pauses
	int offset = 0;
	for (int i=0; i<gnotes.num; i++){
		if (gnotes[i].offset > offset){
			GuitarNote pause;
			pause.offset = offset;
			pause.length = gnotes[i].offset - offset;
			gnotes.insert(pause, i);
		}
		offset = gnotes[i].offset + gnotes[i].length;
	}

	foreach(GuitarNote &n, gnotes)
		n.detune();

	return gnotes;
}

void FormatGuitarPro::write_measure(Track *t, Bar &b)
{
	msg_db_f("measure", 1);
	//msg_write(format("%x", f->GetPos()));

	//msg_write("-----");
	Array<GuitarNote> gnotes = create_guitar_notes(t, b);

	f->WriteInt(gnotes.num); // beats
	foreach(GuitarNote &n, gnotes)
		write_beat(n.pitch, n.string, n.length);

	if (version >= 500) // second voice
		f->WriteInt(0);

	if (version >= 500)
		f->WriteByte(0);
}

int FormatGuitarPro::read_beat(GpTrack &t, GpMeasure &m, int start)
{
	msg_db_f("beat", 2);
	int flags = f->ReadByte();
	if ((flags & 0x40) != 0)
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
	//msg_write(format("0x%x  0x%x  %d   %d", flags, stringFlags, duration, t.stringCount));
	for (int i = 6; i >= 0; i--) {
		if ((stringFlags & (1 << i)) != 0 && (6 - i) < t.stringCount) {
			//TGString string = track.getString( (6 - i) + 1 ).clone(getFactory());
			read_note(t, (6 - i), start, duration);
			//TGNote note = readNote(string, track,effect.clone(getFactory()));
			//voice.addNote(note);
		}
	}
	if (version >= 500){
		f->ReadByte();
		int r = f->ReadByte();
		if ((r & 0x08) != 0)
			f->ReadByte();
	}
	return duration;
}

void FormatGuitarPro::write_beat(Array<int> &pitch, Array<int> &string, int length)
{
	msg_db_f("beat", 2);

	bool is_pause = (pitch.num == 0);

	bool dotted = (length == 3) or (length == 6) or (length == 12);
	if (dotted)
		length = (length * 2) / 3;

	f->WriteByte((dotted ? 0x01 : 0x00) | (is_pause ? 0x40 : 0x00));

	if (is_pause)
		f->WriteByte(0x02);

	if (length == 1)
		f->WriteByte(2);
	else if (length == 2)
		f->WriteByte(1);
	else if (length == 4)
		f->WriteByte(0);
	else if (length == 8)
		f->WriteByte(255);
	else if (length == 16)
		f->WriteByte(254);
	else{
		f->WriteByte(0);
		tsunami->log->error("invalid gp length: " + i2s(length));
	}

	if (pitch.num > 0){
		int sflags = 0;
		for (int i=0; i<string.num; i++)
			sflags |= (0x02 << string[i]);
		f->WriteByte(sflags);


		for (int i=string.num-1; i>=0; i--){
			f->WriteByte(0x20);
			f->WriteByte(1);
			f->WriteByte(pitch[i]);
			if (version >= 500){
				f->WriteByte(0);
			}
		}

	}else{
		f->WriteByte(0);
	}
	/*msg_write(format("0x%x  0x%x  %d   %d", flags, stringFlags, duration, t.stringCount));
	for (int i = 6; i >= 0; i--) {
		if ((stringFlags & (1 << i)) != 0 && (6 - i) < t.stringCount) {
			//TGString string = track.getString( (6 - i) + 1 ).clone(getFactory());
			read_note(t, (6 - i), start, duration);
			//TGNote note = readNote(string, track,effect.clone(getFactory()));
			//voice.addNote(note);
		}
	}*/
	if (version >= 500){
		f->WriteByte(0);
		f->WriteByte(0);
	}
}

void FormatGuitarPro::read_chord()
{
	msg_db_f("chord", 0);
	int type = f->ReadByte();
	if (((type & 0x01) == 0) and (version < 500)){
		string name = read_str41(f);
		msg_write("chord: " + name);
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
		msg_write("chord: " + name);
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

void FormatGuitarPro::read_note(GpTrack &t, int string_no, int start, int length)
{
	msg_db_f("note", 2);
	MidiNote n;
	n.range = Range(start, length);
	n.volume = 1;
	n.pitch = -1;
	int flags = f->ReadByte();
	if ((flags & 0x20) != 0) {
		int noteType = f->ReadByte();
	}
	if (((flags & 0x01) != 0) and (version < 500))
		f->SetPos(2, false);
	if ((flags & 0x10) != 0)
		n.volume = 0.1f + 0.9f * (float)f->ReadByte() / 10.0f;
	if ((flags & 0x20) != 0) {
		int fret = f->ReadByte();
		int value = fret;
		if ((string_no >= 0) && (string_no < t.tuning.num))
			value = fret + t.tuning[string_no];
		//msg_write(format("%d/%d -> %d", string_no, fret, value));
		n.pitch = value;
	}
	if ((flags & 0x80) != 0)
		f->SetPos(2, false);
	if (version >= 500){
		if ((flags & 0x01) != 0)
			f->SetPos(8, false);
		f->ReadByte();
	}
	if ((flags & 0x08) != 0) {
		read_note_fx();
	}
	if (n.volume > 1)
		n.volume = 1;
	if (n.pitch >= 0)
		t.t->addMidiNote(n);
}

void FormatGuitarPro::read_note_fx()
{
	msg_db_f("note fx", 3);
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
		if (version >= 500)
			f->ReadByte();
	}
	if ((flags2 & 0x04) != 0){
		// tremolo picking
		f->ReadByte();
	}
	if ((flags2 & 0x08) != 0)
		f->ReadByte(); // slide
	if ((flags2 & 0x10) != 0)
		int type = f->ReadByte(); // harmonic
	if ((flags2 & 0x20) != 0) {
		// trill
		int fret = f->ReadByte();
		int period = f->ReadByte();
	}
}

int FormatGuitarPro::read_duration(int flags, GpMeasure &m)
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

void FormatGuitarPro::read_mix_change()
{
	msg_db_f("mix", 1);
	f->ReadByte();
	if (version >= 500){
		for (int i=0; i<4; i++)
			f->ReadInt();
	}
	int volume = (signed char)f->ReadByte();
	int pan = (signed char)f->ReadByte();
	int chorus = (signed char)f->ReadByte();
	int reverb = (signed char)f->ReadByte();
	int phaser = (signed char)f->ReadByte();
	int tremolo = (signed char)f->ReadByte();
	if (version > 500)
		read_str41(f);
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
		if (version > 500)
			f->ReadByte();
	}
	if (version >= 400)
		f->ReadByte();
	if (version >= 500)
		f->ReadByte();
	if (version > 500){
		read_str41(f);
		read_str41(f);
	}
}

void FormatGuitarPro::read_beat_fx()
{
	msg_db_f("beat fx", 2);
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
