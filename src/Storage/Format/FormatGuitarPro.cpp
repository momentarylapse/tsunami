/*
 * FormatGuitarPro.cpp
 *
 *  Created on: 01.10.2014
 *      Author: michi
 */

#include "FormatGuitarPro.h"

#include "../../Data/Rhythm/Bar.h"

const int BEAT_PARTITION = 12;

FormatDescriptorGuitarPro::FormatDescriptorGuitarPro() :
	FormatDescriptor("GuitarPro", "gp3,gp4,gp5", Flag::MIDI | Flag::READ | Flag::WRITE | Flag::MULTITRACK)
{
}


static void write_str1(File *f, const string &s)
{
	f->write_byte(s.num);
	f->write_buffer(s.data, s.num);
}

static void write_str1c(File *f, const string &s, int size)
{
	f->write_byte(s.num);
	string t = s;
	t.resize(size);
	f->write_buffer(t.data, size);
}

static void write_str4(File *f, const string &s)
{
	f->write_int(s.num);
	f->write_buffer(s.data, s.num);
}

static void write_str41(File *f, const string &s)
{
	f->write_int(s.num + 1);
	write_str1(f, s);
}

static string read_str1(File *f)
{
	int l = f->read_byte();
	//msg_write(l);
	string s;
	s.resize(l);
	f->read_buffer(s.data, l);
	return s;
}

static string read_str1c(File *f, int size)
{
	int l = f->read_byte();
	string s;
	s.resize(size);
	f->read_buffer(s.data, size);
	s.resize(l);
	return s;
}

static string read_str4(File *f)
{
	int l = f->read_int();
	string s;
	s.resize(l);
	f->read_buffer(s.data, l);
	return s;
}

static string read_str41(File *f)
{
	int l = f->read_int();
	/*msg_write(l);
	string s;
	s.resize(l);
	f->read_buffer(s.data, l);
	return s;*/
	return read_str1(f);
}

void FormatGuitarPro::saveSong(StorageOperationData *_od)
{
	od = _od;
	a = od->song;
	char data[16];

	f = FileCreate(od->filename);
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

	//Array<Track*> tracks;
	for (Track *t : a->tracks)
		if (t->type == t->Type::MIDI){
			GpTrack tt;
			tt.is_drum = (t->instrument.type == Instrument::Type::DRUMS);
			tt.midi_instrument = t->instrument.midi_no();
			tt.tuning = t->instrument.string_pitch;
			if (tt.tuning.num == 0)
				tt.tuning = Instrument(Instrument::Type::ELECTRIC_GUITAR).string_pitch;
			tt.t = t;
			tracks.add(tt);
		}
	Array<Bar*> bars = a->bars.getBars(Range::ALL);
	tempo = 90;
	if (bars.num > 0)
		tempo = (bars[0]->bpm(a->sample_rate) + 0.5f);


	write_info();

	if (version < 500)
		f->write_byte(0); // tripplet feel

	if (version >= 400)
		write_lyrics();
	if (version > 500)
		write_eq();
	if (version >= 500)
		write_page_setup();

	if (version >= 500)
		write_str41(f, "");
	f->write_int(tempo);

	if (version > 500)
		f->write_byte(0); // ???

	if (version >= 400){
		f->write_byte(0); // key signature
		f->write_buffer(data, 3);
		f->write_byte(0); // octave
	}else{
		f->write_int(0); // key
	}

	write_channels();

	if (version >= 500)
		f->seek(42);

	f->write_int(bars.num);
	f->write_int(tracks.num);
	for (Bar *b : bars)
		write_measure_header(b);
	foreachi(GpTrack &t, tracks, i)
		write_track(&t, i);

	if (version >= 500)
		f->write_byte(0);

	for (Bar *b : bars){
		for (GpTrack &t : tracks)
			write_measure(&t, b);
	}
	delete(f);
}

void FormatGuitarPro::loadSong(StorageOperationData *_od)
{
	od = _od;
	a = od->song;
	char data[16];
	tracks.clear();
	measures.clear();

	try{
		f = FileOpen(od->filename);

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
			int tripplet_feel = f->read_byte();

		if (version >= 400)
			read_lyrics();
		if (version > 500)
			read_eq();
		if (version >= 500)
			read_page_setup();

		if (version >= 500)
			msg_write("tempo: " + read_str41(f));
		tempo = f->read_int();
		//msg_write(tempo);

		if (version > 500)
			f->read_byte();

		if (version >= 400){
			f->read_byte(); // key signature
			f->read_buffer(data, 3);
			f->read_byte(); // octave
		}else{
			f->read_int(); // key
		}

		read_channels();

		if (version >= 500)
			f->seek(42);

		int num_measures = f->read_int();
		int num_tracks = f->read_int();
		msg_write(format("measures: %d   tracks: %d", num_measures, num_tracks));
		for (int i=0; i<num_measures; i++)
			read_measure_header();
		a->addTrack(Track::Type::TIME);
		for (int i=0; i<num_tracks; i++)
			read_track();

		if (version >= 500)
			f->read_byte();

		int offset = 0;
		for (int i = 0; i < num_measures; i++){
			for (int j = 0; j < num_tracks; j++)
				read_measure(measures[i], tracks[j], offset);
			if (measures[i].marker.num > 0)
				a->tracks[0]->addMarker(Range(offset, 0), measures[i].marker);
			offset += (int)(a->sample_rate * 60.0f / (float)tempo * 4.0f * (float)measures[i].numerator / (float)measures[i].denominator);
			a->addBar(-1, tempo, measures[i].numerator, 1, false);
		}

	}catch(Exception &e){
		od->error(e.message());
	}

	if (f)
		FileClose(f);
}

void FormatGuitarPro::read_info()
{
	string s;
	s = read_str41(f);
	if (s.num > 0)
		a->addTag("title", s);
	read_str41(f);
	s = read_str41(f);
	if (s.num > 0)
		a->addTag("artist", s);
	s = read_str41(f);
	if (s.num > 0)
		a->addTag("album", s);
	if (version >= 500){
		s = read_str41(f);
		if (s.num > 0)
			a->addTag("lyricist", s);
	}
	s = read_str41(f);
	if (s.num > 0)
		a->addTag("author", s);
	s = read_str41(f);
	if (s.num > 0)
		a->addTag("copyright", s);
	s = read_str41(f);
	if (s.num > 0)
		a->addTag("writer", s);
	read_str41(f);
	int n = f->read_int();
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
	f->write_int(0); // #comments
}

void FormatGuitarPro::read_lyrics()
{
	int lyrics_track = f->read_int();
	//msg_write(lyrics_track);

	f->read_int();
	read_str4(f);
	for (int i=0; i<4; i++){
		f->read_int();
		read_str4(f);
	}
}

void FormatGuitarPro::write_lyrics()
{
	f->write_int(0); // lyrics track
	//msg_write(lyrics_track);

	f->write_int(0);
	write_str4(f, "");
	for (int i=0; i<4; i++){
		f->write_int(0);
		write_str4(f, "");
	}
}

void FormatGuitarPro::read_channels()
{
	for (int i = 0; i < 64; i++) {
		channels[i].instrument = f->read_int(); // program
		channels[i].volume = f->read_byte(); // volume
		channels[i].balance = f->read_byte(); // balance
		f->read_byte(); // chorus
		f->read_byte(); // reverb
		f->read_byte(); // phaser
		f->read_byte(); // tremolo
		//if (i == 9) -> percussion
		f->seek(2);
	}
}

void FormatGuitarPro::write_channels()
{
	for (int i = 0; i < 64; i++) {
		int instrument = 0;
		if (i < tracks.num)
			instrument = tracks[i].midi_instrument;
		f->write_int(instrument); // program
		f->write_byte(100); // volume
		f->write_byte(0); // balance
		f->write_byte(0); // chorus
		f->write_byte(0); // reverb
		f->write_byte(0); // phaser
		f->write_byte(0); // tremolo
		//if (i == 9) -> percussion
		f->seek(2);
	}
}

void FormatGuitarPro::read_eq()
{
	f->read_int(); // master volume
	f->read_int();
	for (int i = 0; i < 11; i++)
		f->read_byte(); // eq
}

void FormatGuitarPro::write_eq()
{
	f->write_int(100); // master volume
	f->write_int(0);
	for (int i = 0; i < 11; i++)
		f->write_byte(100); // eq
}

void FormatGuitarPro::read_page_setup()
{
	f->read_int();
	f->read_int();
	f->read_int();
	f->read_int();
	f->read_int();
	f->read_int();
	f->read_int();
	f->read_word();
	for (int i=0; i<10; i++)
		read_str41(f);
}

void FormatGuitarPro::write_page_setup()
{
	f->write_int(0);
	f->write_int(0);
	f->write_int(0);
	f->write_int(0);
	f->write_int(0);
	f->write_int(0);
	f->write_int(0);
	f->write_word(0);
	for (int i=0; i<10; i++)
		write_str41(f, "");
}

void FormatGuitarPro::read_measure_header()
{
	GpMeasure m;
	if (measures.num > 0){
		m = measures.back();
		m.marker = "";
	}else{
		m.numerator = 1;
		m.denominator = 1;
	}
	int flags = f->read_byte();
	if ((flags & 0x01) != 0)
		m.numerator = f->read_byte();
	if ((flags & 0x02) != 0)
		m.denominator = f->read_byte();
	if ((flags & 0x08) != 0)
		f->read_byte(); // repeat close
	if (((flags & 0x10) != 0) and (version < 500))
		f->read_byte(); // repeat alternative
	if ((flags & 0x20) != 0){
		m.marker = read_str41(f);
		f->read_int(); // color
	}
	if (((flags & 0x10) != 0) and (version >= 500))
		f->read_byte(); // repeat alternative
	if ((flags & 0x40) != 0){
		f->read_byte(); // key
		f->read_byte();
	}
	if (version >= 500){
		if ((flags & 0x03) != 0)
			f->read_int(); // beam 8 notes
		if ((flags & 0x10) == 0)
			f->read_byte(); // repeat alternative
		f->read_byte(); // triplet feel
		f->read_byte();
	}
	measures.add(m);
}

void FormatGuitarPro::write_measure_header(Bar *b)
{
	f->write_byte(0x03);
	f->write_byte(b->num_beats);
	f->write_byte(4);
	if (version >= 500){
		//if ((flags & 0x03) != 0)
			f->write_int(0); // beam 8 notes
		//if ((flags & 0x10) == 0)
			f->write_byte(0); // repeat alternative
		f->write_byte(0); // triplet feel
		f->write_byte(0);
	}
}

void FormatGuitarPro::read_track()
{
	f->read_byte();
	GpTrack tt;
	tt.t = a->addTrack(Track::Type::MIDI);
	tt.t->setName(read_str1c(f, 40));
	int stringCount = f->read_int();
	for (int i=0; i<7; i++){
		int tuning = f->read_int(); // tuning
		tt.tuning.add(tuning);
	}
	tt.tuning.resize(stringCount);
	tracks.add(tt);
	int port = f->read_int();
	int channel = f->read_int();
	int channel_fx = f->read_int();
	Instrument instrument = Instrument(Instrument::Type::DRUMS);
	if (channel != 10)
		instrument.set_midi_no(channels[(port-1) * 16 + (channel-1)].instrument);

	instrument.string_pitch = tt.tuning;
	instrument.string_pitch.reverse();
	for (int i=instrument.string_pitch.num-1; i>=0; i--)
		if (instrument.string_pitch[i] <= 0)
			instrument.string_pitch.erase(i);
	tt.t->setInstrument(instrument);

	f->read_int();
	f->read_int(); // offset
	f->read_int(); // color
	if (version > 500){
		f->seek(49);
		read_str41(f);
		read_str41(f);
	}
	if (version == 500)
		f->seek(45);
}

void FormatGuitarPro::write_track(GpTrack *t, int index)
{
	f->write_byte(0);
	GpTrack tt;
	write_str1c(f, t->t->name, 40);
	// tuning
	f->write_int(t->tuning.num); // string count
	for (int i=min(t->tuning.num, 7)-1; i>=0; i--)
		f->write_int(t->tuning[i]);
	for (int i=t->tuning.num; i<7; i++)
		f->write_int(0);

	// port / channel
	if (t->is_drum){
		f->write_int(1);
		f->write_int(10);
	}else{
		f->write_int((index / 16) + 1);
		f->write_int((index % 16) + 1);
	}
	f->write_int(0); // channel fx

	f->write_int(24);
	f->write_int(0); // offset
	f->write_int(0); // color
	if (version > 500){
		f->seek(49);
		write_str41(f, "");
		write_str41(f, "");
	}
	if (version == 500)
		f->seek(45);
}

void FormatGuitarPro::read_measure(GpMeasure &m, GpTrack &t, int offset)
{
	//msg_write(format("m %x", f->GetPos()));

	int num_voices = 1;
	if (version >= 500)
		num_voices = 2;

	for (int v=0; v<num_voices; v++){

		int num_beats = f->read_int();
		//msg_write(num_beats);
		if (num_beats > 1000)
			throw Exception("too many beats... " + i2s(num_beats));
		for (int i=0; i<num_beats; i++){
			int length = read_beat(t, m, offset);
			offset += length;
		}
	}

	if (version >= 500)
		f->read_byte();
}

struct GuitarNote
{
	int offset, length;
	Array<int> pitch;
	Array<int> string;
	void detune(Array<int> &tuning)
	{
		// sort ascending
		for (int i=0; i<pitch.num; i++)
			for (int j=i+1; j<pitch.num; j++)
				if (pitch[i] >= pitch[j]){
					int t = pitch[i];
					pitch[i] = pitch[j];
					pitch[j] = t;
				}
		Array<int> p0 = pitch;

		string.resize(pitch.num);
		int highest_available_string = tuning.num-1;
		for (int n=pitch.num-1; n>=0; n--){
			bool found = false;
			for (int i=highest_available_string; i>=0; i--){
				if (pitch[n] >= tuning[i]){
					string[n] = i;
					pitch[n] -= tuning[i];
					highest_available_string = i-1;
					found = true;
					break;
				}
			}
			if (!found){
				msg_error("could not detune:");
				msg_write(ia2s(p0));
				msg_write(ia2s(tuning));
				pitch.erase(n);
				string.erase(n);
			}
		}
	}
};

Array<int> decompose_time(int length)
{
	Array<int> t;

	// triplets * 2^n
	while ((length % 3) != 0){
		int largest = 2;
		for (int i=2; i<=length; i*=2)
			if (i <= length)
				largest = i;

		t.add(largest);
		length -= largest;
		//msg_error("non decomposable time: " + i2s(length));
	}

	// non-triplets
	while (length > 0){
		int largest = 3;
		for (int i=3; i<=length; i*=2){
			// dotted
			if (i/2*3 <= length)
				largest = i/2*3;
			// normal
			else if (i <= length)
				largest = i;
		}
		t.add(largest);
		length -= largest;
	}
	return t;
}

Array<GuitarNote> create_guitar_notes(FormatGuitarPro::GpTrack *t, Bar *b)
{
	// samples per 16th / 3
	float spu = (float)b->range().length / (float)b->num_beats / (float)BEAT_PARTITION;

	MidiNoteBufferRef notes = t->t->midi.getNotes(b->range());
	Array<GuitarNote> gnotes;

	for (MidiNote *n : notes){
		Range r = n->range and b->range();
		GuitarNote gn;
		gn.offset = int((float)(r.offset - b->range().offset) / spu + 0.5f);
		gn.length = int((float)(r.end() - b->range().offset) / spu + 0.5f) - gn.offset;
		gn.pitch.add(n->pitch);
		if (gn.length == 0)
			continue;
		if (gn.offset < b->num_beats * BEAT_PARTITION)
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

	// decompose evil timings
	for (int i=gnotes.num-1; i>=0; i--){
		Array<int> t = decompose_time(gnotes[i].length);
		if (t.num > 1){
			int offset = gnotes[i].offset;
			for (int j=0; j<t.num; j++){
				GuitarNote n;
				n.pitch = gnotes[i].pitch;
				n.offset = offset;
				n.length = t[j];
				gnotes.insert(n, i + j + 1);
				offset += t[j];
			}
			gnotes.erase(i);
		}
		offset = gnotes[i].offset + gnotes[i].length;
	}

	for (GuitarNote &n : gnotes)
		n.detune(t->tuning);

	return gnotes;
}

void FormatGuitarPro::write_measure(GpTrack *t, Bar *b)
{
	//msg_write(format("m %x", f->GetPos()));

	int bpm = (b->bpm(t->t->song->sample_rate) + 0.5f);
	bool update_tempo = (bpm != tempo);
	tempo = bpm;

	//msg_write("-----");
	Array<GuitarNote> gnotes = create_guitar_notes(t, b);

	// empty? -> at least fill in a long break
	if (gnotes.num == 0){
		GuitarNote g;
		g.length = BEAT_PARTITION;
		while (true){
			if (g.length * 2 > BEAT_PARTITION * b->num_beats)
				break;
			g.length *= 2;
		}
		g.offset = 0;
		gnotes.add(g);
	}

	// beats
	f->write_int(gnotes.num);
	foreachi(GuitarNote &n, gnotes, i)
		write_beat(t, n.pitch, n.string, n.length, update_tempo and (i == 0));

	if (version >= 500) // second voice
		f->write_int(0);

	if (version >= 500)
		f->write_byte(0);
}

int FormatGuitarPro::read_beat(GpTrack &t, GpMeasure &m, int start)
{
	int flags = f->read_byte();
	if ((flags & 0x40) != 0)
		f->read_byte();

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
	int stringFlags = f->read_byte();
	//msg_write(format("0x%x  0x%x  %d   %d", flags, stringFlags, duration, t.stringCount));
	for (int i = 6; i >= 0; i--)
		if ((stringFlags & (1 << i)) != 0 and (6 - i) < t.tuning.num)
			read_note(t, (6 - i), start, duration);

	if (version >= 500){
		f->read_byte();
		int r = f->read_byte();
		if ((r & 0x08) != 0)
			f->read_byte();
	}
	return duration;
}

void FormatGuitarPro::write_beat(GpTrack *t, Array<int> &pitch, Array<int> &string, int length, bool update_tempo)
{
	if (length <= 1){
		od->error("write_beat: evil length: " + i2s(length));
		length = 3; // quick and dirty fix :P
	}

	bool is_pause = (pitch.num == 0);

	bool tripple = ((length % 3) != 0);
	if (tripple)
		length = length / 2;
	else
		length /= 3;

	bool dotted = (length == 3) or (length == 6) or (length == 12) or (length == 24);
	if (dotted)
		length = (length * 2) / 3;

	f->write_byte((dotted ? 0x01 : 0x00) | (tripple ? 0x20 : 0x00) | (is_pause ? 0x40 : 0x00) | (update_tempo ? 0x10 : 0x00));

	if (is_pause)
		f->write_byte(0x02);

	if (length == 1)
		f->write_byte(2);
	else if (length == 2)
		f->write_byte(1);
	else if (length == 4)
		f->write_byte(0);
	else if (length == 8)
		f->write_byte(255);
	else if (length == 16)
		f->write_byte(254);
	else{
		f->write_byte(0);
		od->error(format("invalid gp length: %d", length));
	}

	if (tripple)
		f->write_int(3);

	if (update_tempo)
		write_mix_change_tempo();

	if (pitch.num > 0){
		int sflags = 0;
		for (int i=0; i<string.num; i++)
			sflags |= (0x01 << (string[i] + 7 - t->tuning.num));
		f->write_byte(sflags);


		for (int i=string.num-1; i>=0; i--){
			f->write_byte(0x20);
			f->write_byte(1);
			f->write_byte(pitch[i]);
			if (version >= 500){
				f->write_byte(0);
			}
		}

	}else{
		f->write_byte(0);
	}
	/*msg_write(format("0x%x  0x%x  %d   %d", flags, stringFlags, duration, t.stringCount));
	for (int i = 6; i >= 0; i--) {
		if ((stringFlags & (1 << i)) != 0 and (6 - i) < t.stringCount) {
			//TGString string = track.getString( (6 - i) + 1 ).clone(getFactory());
			read_note(t, (6 - i), start, duration);
			//TGNote note = readNote(string, track,effect.clone(getFactory()));
			//voice.addNote(note);
		}
	}*/
	if (version >= 500){
		f->write_byte(0);
		f->write_byte(0);
	}
}

void FormatGuitarPro::read_chord()
{
	int type = f->read_byte();
	if (((type & 0x01) == 0) and (version < 500)){
		string name = read_str41(f);
		msg_write("chord: " + name);
		int first_fret = f->read_int();
		if (first_fret != 0){
			for (int i=0; i<6; i++) {
				int fret = f->read_int();
				/*if(i < chord.countStrings()){
					chord.addFretValue(i,fret);
				}*/
			}
		}
	}else{
		f->seek(16);
		string name = read_str1c(f, 21);
		msg_write("chord: " + name);
		f->seek(4);
		int first_fret = f->read_int();
		for (int i=0; i<7; i++){
			int fret = f->read_int();
			/*if(i < chord.countStrings()){
				chord.addFretValue(i,fret);
			}*/
		}
		f->seek(32);
	}
}

void FormatGuitarPro::read_note(GpTrack &t, int string_no, int start, int length)
{
	MidiNote *n = new MidiNote(Range(start, length), -1, 1);
	int flags = f->read_byte();
	if ((flags & 0x20) != 0) {
		int noteType = f->read_byte();
	}
	if (((flags & 0x01) != 0) and (version < 500))
		f->seek(2);
	if ((flags & 0x10) != 0)
		n->volume = 0.1f + 0.9f * (float)f->read_byte() / 10.0f;
	if ((flags & 0x20) != 0) {
		int fret = f->read_byte();
		int value = fret;
		if ((string_no >= 0) and (string_no < t.tuning.num))
			value = fret + t.tuning[string_no];
		//msg_write(format("%d/%d -> %d", string_no, fret, value));
		n->pitch = value;
	}
	if ((flags & 0x80) != 0)
		f->seek(2);
	if (version >= 500){
		if ((flags & 0x01) != 0)
			f->seek(8);
		f->read_byte();
	}
	if ((flags & 0x08) != 0) {
		read_note_fx();
	}
	if (n->volume > 1)
		n->volume = 1;
	if (n->pitch >= 0)
		t.t->addMidiNote(n);
}

void FormatGuitarPro::read_note_fx()
{
	int flags1 = f->read_byte();
	int flags2 = 0;
	if (version >= 400)
		flags2 = f->read_byte();
	if ((flags1 & 0x01) != 0) {
		// bend
		f->seek(5);
		int points = f->read_int();
		for (int i=0; i<points; i++){
			int position = f->read_int();
			int value = f->read_int();
			f->read_byte();
		}
	}
	if ((flags1 & 0x10) != 0) {
		// grace
		int fret = f->read_byte();
		int volume = f->read_byte();
		int transition = f->read_byte();
		int duration = f->read_byte();
		if (version >= 500)
			f->read_byte();
	}
	if ((flags2 & 0x04) != 0){
		// tremolo picking
		f->read_byte();
	}
	if ((flags2 & 0x08) != 0)
		f->read_byte(); // slide
	if ((flags2 & 0x10) != 0)
		int type = f->read_byte(); // harmonic
	if ((flags2 & 0x20) != 0) {
		// trill
		int fret = f->read_byte();
		int period = f->read_byte();
	}
}

int FormatGuitarPro::read_duration(int flags, GpMeasure &m)
{
	int v = (signed char)f->read_byte();
	float value = 16.0f / (float)(int)(1 << (v + 4));
	bool dotted = ((flags & 0x01) != 0);
	if (dotted)
		value *= 1.5f;
	if ((flags & 0x20) != 0) {
		int divisionType = f->read_int();
		switch (divisionType) {
		case 3:
			value *= 2.0f/3.0f;
			break;
		case 5:
			value *= 4.0f/5.0f;
			break;
		case 6:
			value *= 4.0f/6.0f; // wtf?
			break;
		case 7:
			value *= 4.0f/7.0f;
			break;
		case 9:
			value *= 8.0f/9.0f;
			break;
		case 10:
			value *= 8.0f/10.0f; // ?
			break;
		case 11:
			value *= 8.0f/11.0f;
			break;
		case 12:
			value *= 8.0f/12.0f; // ?
			break;
		default:
			od->error(format("unknown beat division: %d", divisionType));
		}
	}
	return a->sample_rate * value * 60.0f / (float)tempo;
}

void FormatGuitarPro::read_mix_change()
{
	f->read_byte(); // instrument
	if (version >= 500){
		for (int i=0; i<4; i++)
			f->read_int();
	}
	int volume = (signed char)f->read_byte();
	int pan = (signed char)f->read_byte();
	int chorus = (signed char)f->read_byte();
	int reverb = (signed char)f->read_byte();
	int phaser = (signed char)f->read_byte();
	int tremolo = (signed char)f->read_byte();
	if (version >= 500)
		read_str41(f);
	int tempoValue = f->read_int();
	//msg_write(format("%d %d %d %d %d %d %d", volume, pan, chorus, reverb, phaser, tremolo, tempoValue));
	if (volume >= 0)
		f->read_byte();
	if (pan >= 0)
		f->read_byte();
	if (chorus >= 0)
		f->read_byte();
	if (reverb >= 0)
		f->read_byte();
	if (phaser >= 0)
		f->read_byte();
	if (tremolo >= 0)
		f->read_byte();
	if (tempoValue >= 0){
		tempo = tempoValue;
		f->read_byte();
		if (version > 500)
			f->read_byte();
	}
	if (version >= 400)
		f->read_byte();
	if (version >= 500)
		f->read_byte();
	if (version > 500){
		read_str41(f);
		read_str41(f);
	}
}

void FormatGuitarPro::write_mix_change_tempo()
{
	f->write_byte(0xff);
	if (version >= 500){
		for (int i=0; i<4; i++)
			f->write_int(0);
	}
	f->write_byte(0xff); // volume
	f->write_byte(0xff); // pan
	f->write_byte(0xff); // chorus
	f->write_byte(0xff); // reverb
	f->write_byte(0xff); // phaser
	f->write_byte(0xff); // tremolo
	if (version >= 500)
		write_str41(f, "");

	f->write_int(tempo);
	f->write_byte(0);
	if (version > 500)
		f->write_byte(0);

	if (version >= 400)
		f->write_byte(0);
	if (version >= 500)
		f->write_byte(0);
	if (version > 500){
		write_str41(f, "");
		write_str41(f, "");
	}
}

void FormatGuitarPro::read_beat_fx()
{
	int flags1 = f->read_byte();
	int flags2 = 0;
	if (version >= 400)
		flags2 = f->read_byte();
	if ((flags1 & 0x20) != 0) {
		int effect = f->read_byte();
		if (version < 400)
			f->read_int();
	}
	if ((flags2 & 0x04) != 0) {
		// tremolo
		f->seek(5);
		int points = f->read_int();
		//msg_write(points);
		for (int i=0; i<points; i++){
			int position = f->read_int();
			int value = f->read_int();
			f->read_byte();
		}
	}
	if ((flags1 & 0x40) != 0) {
		int strokeDown = f->read_byte();
		int strokeUp = f->read_byte();
	}
	if ((flags2 & 0x02) != 0)
		f->read_byte();
}
