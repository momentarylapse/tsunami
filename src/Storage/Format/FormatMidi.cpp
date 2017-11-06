/*
 * FormatMidi.cpp
 *
 *  Created on: 17.02.2013
 *      Author: michi
 */

#include "FormatMidi.h"

FormatDescriptorMidi::FormatDescriptorMidi() :
	FormatDescriptor("Midi", "mid,midi", FLAG_MIDI | FLAG_MULTITRACK | FLAG_READ | FLAG_WRITE)
{
}

static string read_chunk_name(File *f)
{
	string s;
	s.resize(4);
	*(int*)s.data = f->ReadInt();
	return s;
}

static void write_chunk_name(File *f, const string &name)
{
	string s = name;
	s.resize(4);
	f->WriteBuffer(s.data, 4);
}

static int int_reverse(int i)
{
	return ((i & 255) << 24) + (((i >> 8) & 255) << 16) + (((i >> 16) & 255) << 8) + ((i >> 24) & 255);
}

static int int16_reverse(int i)
{
	return ((i & 255) << 8) + ((i >> 8) & 255);
}

static int read_int(File *f)
{
	int i = f->ReadInt();
	return int_reverse(i);
}

static int write_int(File *f, int i)
{
	f->WriteInt(int_reverse(i));
}

static int read_var(File *f)
{
	unsigned char c0;
	int i = 0;
	do{
		c0 = f->ReadByte();
		i = (c0 & 127) + (i << 7);
	}while ((c0 & 128) > 0);
	return i;
}

static void write_var(File *f, int i)
{
	for (int offset = 0; offset < 32; offset += 7){
		unsigned char c = (i >> offset) & 0x7f;
		bool more_data = ((i >> (offset + 7)) != 0);
		if (more_data)
			c |= 0x80;
		f->WriteByte(c);
		if (!more_data)
			break;
	}
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

void FormatMidi::loadSong(StorageOperationData *od)
{
	File *f = NULL;
	try{
		f = FileOpen(od->filename);
		if (!f)
			throw string("can not open file");
		f->SetBinaryMode(true);

		string hn = read_chunk_name(f);
		int hsize = read_int(f);
		if (hn != "MThd")
			throw string("midi header is not \"MTHd\": " + hn);
		//msg_write(hn + " " + i2s(hsize));
		int format_type = int16_reverse(f->ReadWord());
		int num_tracks = int16_reverse(f->ReadWord());
		int time_division = int16_reverse(f->ReadWord());
		if (format_type > 2)
			throw format("only format type 1 allowed: %d", format_type);
		msg_write(format("tracks: %d", num_tracks));
		msg_write(format("time division: %d", time_division));

		if (hsize > 6)
			f->SetPos(hsize - 6, false);

		int ticks_per_beat;

		if ((time_division & 0x8000) > 0){

		}else{
			ticks_per_beat = time_division;
		}

		// beat = quarter note
		int mpqn = 500000; // micro s/beat = 120 beats/min;

		int numerator = 4;
		int denominator = 4;
		int last_bar = 0;

		od->song->addTrack(Track::TYPE_TIME);

		for (int i=0; i<num_tracks; i++){
			string tn = read_chunk_name(f);
			int tsize = read_int(f);
			int pos0 = f->GetPos();
			if (tn != "MTrk")
				throw string("midi track header is not \"MTrk\": " + tn);
			//msg_write("----------------------- track");

			Map<int, MidiEventBuffer> events;
			string track_name;
			int last_status = 0;

			int offset = 0;
			while(f->GetPos() < pos0 + tsize){
				int v = read_var(f);
				int dt = (double)v * (double)mpqn / 1000000.0 * (double)od->song->sample_rate / (double)ticks_per_beat;
				offset += dt;
				while (offset > last_bar){
					od->song->addBar(-1, 60000000.0f / (float)mpqn / 4 * (float)denominator, numerator, 1, false);
					last_bar = od->song->bars.getRange().end();
				}
				int c0 = f->ReadByte();
				if ((c0 & 128) == 0){ // "running status"
					c0 = last_status;
					f->SetPos(-1, false);
				}
				//msg_write(format("  %d %p", v, c0));
				last_status = c0;
				if (c0 == 0xff){
					int type = f->ReadByte();
					//msg_write("meta " + i2s(type));
					int l = read_var(f);
					if (type == 81){
						int a0 = f->ReadByte();
						int a1 = f->ReadByte();
						int a2 = f->ReadByte();
						mpqn = (a0 << 16) + (a1 << 8) + a2;
						//msg_write(format("%.1f bpm", 60000000.0f / (float)mpqn));
					}else if (type == 88){
						int a0 = f->ReadByte();
						int a1 = f->ReadByte();
						int a2 = f->ReadByte();
						int a3 = f->ReadByte();
						numerator = a0;
						denominator = 1<<a1;
						//msg_write(format("time %d %d %d %d", a0, 1<<a1, a2, a3));
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
						events[channel].add(MidiEvent(offset, c1, (float)c2 / 127.0f));
						//msg_write(format("on %d  %d   %d %d", offset, c1, type, channel));
					}else if (type == 8){ // off
						int c1 = f->ReadByte() & 127;
						int c2 = f->ReadByte() & 127;
						events[channel].add(MidiEvent(offset, c1, 0));
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
						od->warn("unhandled midi event " + i2s(type));
				}
			}

			f->SetPos(pos0 + tsize, true);

			if ((events.num > 0) or (i > 0)){
				Array<int> keys = events.keys();
				for (int k : keys){
					Track *t = od->song->addTrack(Track::TYPE_MIDI);
					t->midi = midi_events_to_notes(events[k]);
					t->name = track_name;
				}
			}
		}

		FileClose(f);
	}catch(const string &s){
		if (f)
			FileClose(f);
		od->error(s);
	}
}
void FormatMidi::saveSong(StorageOperationData* od)
{
	File *f = NULL;
	try {
		f = FileCreate(od->filename);
		if (!f)
			throw string("can not create file");

		f->SetBinaryMode(true);
		int num_tracks = 0;
		for (Track *t: od->song->tracks)
			if (t->type == t->TYPE_MIDI)
				num_tracks ++;
		int ticks_per_beat = 16;
		// heaer
		write_chunk_name(f, "MThd");
		write_int(f, 6); // size
		f->WriteWord(int16_reverse(1)); // format
		f->WriteWord(int16_reverse(num_tracks));
		f->WriteWord(int16_reverse(ticks_per_beat));
		// beat = quarter note
		int mpqn = 500000; // micro s/beat = 120 beats/min;
		int numerator = 4;
		int denominator = 4;
		int last_bar = 0;
		for (Track* t : od->song->tracks) {
			if (t->type != t->TYPE_MIDI)
				continue;
			write_chunk_name(f, "MTrk");
			int pos0 = f->GetPos();
			write_int(f, 0); // size
			// track name
			if (t->name.num > 0) {
				write_var(f, 0);
				f->WriteByte(0xff);
				f->WriteByte(0x03);
				write_var(f, t->name.num);
				f->WriteBuffer(t->name.data, t->name.num);
			}
			if (od->song->bars.num > 0) {
				auto b = od->song->bars[0];
				float bpm = 60.0f / ((float) b.length / (float) od->song->sample_rate / b.num_beats);
				mpqn = 60000000.0f / bpm;
				write_var(f, 0);
				f->WriteByte(0xff);
				f->WriteByte(81);
				write_var(f, 0);
				f->WriteByte(mpqn >> 16);
				f->WriteByte(mpqn >> 8);
				f->WriteByte(mpqn >> 0);

				write_var(f, 0);
				f->WriteByte(0xff);
				f->WriteByte(88);
				write_var(f, 0);
				f->WriteByte(b.num_beats);
				f->WriteByte(2); // 1/4
				f->WriteByte(0);
				f->WriteByte(0);
			}
			MidiEventBuffer events = t->midi.getEvents(Range::ALL);
			events.sort();
			int offset = 0;
			for (MidiEvent& e: events) {
				int dt = e.pos - offset;
				int v = (int) (((double) (dt) / (double) (mpqn) * 1000000.0
						/ (double) (od->song->sample_rate)
						* (double) (ticks_per_beat)));
				write_var(f, v);
				offset = e.pos;
				if (e.volume > 0) {
					// on
					f->WriteByte(0x90);
					f->WriteByte((int) (e.pitch));
					f->WriteByte((int) (e.volume) * 127.0f);
				} else {
					// off
					f->WriteByte(0x80);
					f->WriteByte((int) (e.pitch));
					f->WriteByte(0);
				}
			}
			int pos = f->GetPos();
			f->SetPos(pos0, true);
			write_int(f, pos - pos0 - 4);
			f->SetPos(pos, true);
		}
		FileClose(f);
	} catch (const string& s) {
		if (f)
			FileClose(f);

		od->error(s);
	}


}
