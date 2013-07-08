/*
 * FormatMidi.cpp
 *
 *  Created on: 17.02.2013
 *      Author: michi
 */

#include "FormatMidi.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"

FormatMidi::FormatMidi() :
	Format("mid", FLAG_MIDI | FLAG_MULTITRACK)
{
}

FormatMidi::~FormatMidi()
{
}

void FormatMidi::LoadTrack(Track *t, const string &filename, int offset, int level)
{}

void FormatMidi::SaveBuffer(AudioFile *a, BufferBox *b, const string &filename)
{}

static string read_chunk_name(CFile *f)
{
	string s;
	s.resize(4);
	*(int*)s.data = f->ReadInt();
	return s;
}

static int read_int(CFile *f)
{
	int i = f->ReadInt();
	return ((i & 255) << 24) + (((i >> 8) & 255) << 16) + (((i >> 16) & 255) << 8) + ((i >> 24) & 255);
}

static int read_var(CFile *f)
{
	unsigned char c0;
	int i = 0;
	do{
		c0 = f->ReadByte();
		i = (c0 & 127) + (i << 7);
	}while ((c0 & 128) > 0);
	return i;
}

static void note_off(int offset, int note_start[128], int note, int vel, Array<MidiNote> &notes)
{
	//msg_write("off");
	if (note_start[note] < 0)
		return;

	MidiNote n;
	n.range = Range(note_start[note], offset - note_start[note]);
	n.pitch = note;
	n.volume = 1;
	notes.add(n);
	note_start[note] = -1;
}

static void note_on(int offset, int note_start[128], int note, int vel, Array<MidiNote> &notes)
{
	//msg_write("on");
	if (vel == 0)
		note_off(offset, note_start, note, vel, notes);
	else
		note_start[note] = offset;
}

static string ascii2utf8(const string &s)
{
	string r;
	for (int i=0; i<s.num; i++)
		if ((s[i] & 128) == 0){
			r.add(s[i]);
		}else{
			r.add(((s[i] >> 6) & 3) + 0xc0);
			r.add((s[i] & 0x3f) + 0x80);
		}
	return r;
}

void FormatMidi::LoadAudio(AudioFile *a, const string &filename)
{
	CFile *f = NULL;
	try{
		f = OpenFile(filename);
		if (!f)
			throw string("can't open file");
		f->SetBinaryMode(true);

		string hn = read_chunk_name(f);
		int hsize = read_int(f);
		if (hn != "MThd")
			throw string("midi header is not \"MTHd\": " + hn);
		int format_type = f->ReadReversedWord();
		int num_tracks = f->ReadReversedWord();
		int time_division = f->ReadReversedWord();
		if (format_type > 2)
			throw format("only format type 1 allowed: %d", format_type);
		msg_write(num_tracks);
		msg_write(time_division);
		int ticks_per_beat;

		if ((time_division & 0x8000) > 0){

		}else{
			ticks_per_beat = time_division;
		}

		int mpqn = 60000000 / 120;

		for (int i=0; i<num_tracks; i++){
			string tn = read_chunk_name(f);
			int tsize = read_int(f);
			int pos0 = f->GetPos();
			if (tn != "MTrk")
				throw string("midi track header is not \"MTrk\": " + tn);
			//msg_write("----------------------- track");

			int note_start[128];
			for (int i=0;i<128;i++)
				note_start[i] = -1;
			Array<MidiNote> notes;
			string track_name;
			int last_status = 0;

			int offset = 0;
			while(f->GetPos() < pos0 + tsize){
				int v = read_var(f);
				offset += (double)v * (double)mpqn / 1000000.0 * (double)a->sample_rate / (double)ticks_per_beat;
				int c0 = f->ReadByte();
				if ((c0 & 128) == 0){ // "running status"
					c0 = last_status;
					f->SetPos(-1, false);
				}
				//msg_write(format("  %d %p", v, c0));
				last_status = c0;
				if (c0 == 255){
					int type = f->ReadByte();
					//msg_write("meta " + i2s(type));
					int l = read_var(f);
					if (type == 81){
						int a0 = f->ReadByte();
						int a1 = f->ReadByte();
						int a2 = f->ReadByte();
						mpqn = (a0 << 16) + (a1 << 8) + a2;
						msg_write(mpqn);
					}else if (type == 88){
						int a0 = f->ReadByte();
						int a1 = f->ReadByte();
						int a2 = f->ReadByte();
						int a3 = f->ReadByte();
						msg_write(format("time %d %d %d %d", a0, a1, a2, a3));
					}else{
						string t;
						t.resize(l);
						f->ReadBuffer(t.data, l);
						if (type == 3)
							track_name = ascii2utf8(t);
					}
				}else if ((c0 & 0xf0) == 0xf0){
					//msg_write("sys ev");
					int l = read_var(f);
					f->SetPos(l, false);
				}else{
					int type = ((c0 >> 4) & 15);
					int channel = (c0 & 15);
					//msg_write(format("ev %d  %d", type, channel));
					if (type == 9){ // on
						int c1 = f->ReadByte() & 127;
						int c2 = f->ReadByte() & 127;
						note_on(offset, note_start, c1, c2, notes);
					}else if (type == 8){ // off
						int c1 = f->ReadByte() & 127;
						int c2 = f->ReadByte() & 127;
						note_off(offset, note_start, c1, c2, notes);
					}else if (type == 10){ // note after touch
						f->ReadByte();
						f->ReadByte();
					}else if (type == 11){ // controller
						f->ReadByte();
						f->ReadByte();
					}else if (type == 12){ // program change
						f->ReadByte();
					}else if (type == 13){ // channel after touch
						f->ReadByte();
					}else if (type == 14){ // pitch bend
						f->ReadByte();
						f->ReadByte();
					}else
						msg_error("midi event " + i2s(type));
				}
			}

			f->SetPos(pos0 + tsize, true);

			if (notes.num > 0){
				Track *t = a->AddMidiTrack();
				t->midi.append(notes);
				t->name = track_name;
			}
		}

		FileClose(f);
	}catch(const string &s){
		if (f)
			FileClose(f);
		tsunami->log->Error(s);
	}
}

void FormatMidi::SaveAudio(AudioFile *a, const string &filename)
{
}

