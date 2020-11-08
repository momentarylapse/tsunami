/*
 * FormatMidi.cpp
 *
 *  Created on: 17.02.2013
 *      Author: michi
 */

#include "FormatMidi.h"

#include "../../lib/file/file.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Song.h"
#include "../../Data/base.h"
#include "../../Data/Rhythm/Bar.h"

static void dbo(const string &s) {
	//msg_write(s);
}

// https://www.recordingblogs.com/wiki/midi-meta-messages

FormatDescriptorMidi::FormatDescriptorMidi() :
	FormatDescriptor("Midi", "mid,midi", Flag::MIDI | Flag::MULTITRACK | Flag::READ | Flag::WRITE)
{
}

static string read_chunk_name(File *f) {
	string s;
	s.resize(4);
	*(int*)s.data = f->read_int();
	return s;
}

static void write_chunk_name(File *f, const string &name) {
	string s = name;
	s.resize(4);
	f->write_buffer(s);
}

static int int_reverse(int i) {
	return ((i & 255) << 24) + (((i >> 8) & 255) << 16) + (((i >> 16) & 255) << 8) + ((i >> 24) & 255);
}

static int int16_reverse(int i) {
	return ((i & 255) << 8) + ((i >> 8) & 255);
}

static int read_int(File *f)
{
	int i = f->read_int();
	return int_reverse(i);
}

static void write_int(File *f, int i) {
	f->write_int(int_reverse(i));
}

static int read_var(File *f) {
	unsigned char c0;
	int i = 0;
	do {
		c0 = f->read_byte();
		i = (c0 & 127) + (i << 7);
	} while ((c0 & 128) > 0);
	return i;
}

static void write_var(File *f, int i) {
	string s;
	s.add(i & 0x7f);
	i >>= 7;
	if (i >= 0) {
		s.add((i & 0x7f) | 0x80);
		i >>= 7;
	}
	if (i >= 0) {
		s.add((i & 0x7f) | 0x80);
		i >>= 7;
	}
	f->write_buffer(s.reverse());
}

static string ascii2utf8(const string &s) {
	string r;
	for (int i=0; i<s.num; i++)
		if ((s[i] & 0x80) == 0) {
			r.add(s[i]);
		} else {
			r.add(((s[i] >> 6) & 3) + 0xc0);
			r.add((s[i] & 0x3f) + 0x80);
		}
	return r;
}

void FormatMidi::load_song(StorageOperationData *od) {
	File *f = nullptr;
	try {
		f = FileOpen(od->filename);

		string hn = read_chunk_name(f);
		int hsize = read_int(f);
		if (hn != "MThd")
			throw Exception("midi header is not \"MTHd\": " + hn);
		//msg_write(hn + " " + i2s(hsize));
		int format_type = int16_reverse(f->read_word());
		int num_tracks = int16_reverse(f->read_word());
		int time_division = int16_reverse(f->read_word());
		if (format_type > 2)
			throw Exception(format("only format type 1 allowed: %d", format_type));
		dbo(format("tracks: %d", num_tracks));
		dbo(format("time division: %d", time_division));

		if (hsize > 6)
			f->seek(hsize - 6);

		int ticks_per_beat = 4;

		if ((time_division & 0x8000) > 0) {

		} else {
			ticks_per_beat = time_division;
		}

		// beat = quarter note
		int mpqn = 500000; // micro s/beat = 120 beats/min;

		int numerator = 4;
		int denominator = 4;
		int last_bar = 0;

		od->song->add_track(SignalType::BEATS);

		for (int i=0; i<num_tracks; i++) {
			string tn = read_chunk_name(f);
			int tsize = read_int(f);
			int pos0 = f->get_pos();
			if (tn != "MTrk")
				throw Exception("midi track header is not \"MTrk\": " + tn);
			dbo("----------------------- track");

			Map<int, MidiEventBuffer> events;
			string track_name;
			int last_status = 0;

			int moffset = 0;
			while (f->get_pos() < pos0 + tsize) {
				int v = read_var(f);
				dbo(format("offset=%d", v));
				moffset += v;
				int length = (double)v * (double)mpqn / 1000000.0 * (double)od->song->sample_rate / (double)ticks_per_beat;
				int offset = (double)moffset * (double)mpqn / 1000000.0 * (double)od->song->sample_rate / (double)ticks_per_beat;
//				int offset = 0;
				//throw Exception("ahhhhhhhhhhh");
				while (offset > last_bar) {
					int bar_length = (double)numerator * (double)mpqn / 1000000.0 * (double)od->song->sample_rate;
					auto b = BarPattern(bar_length, numerator, max(denominator / 4, 1));
					od->song->add_bar(-1, b, false);
					last_bar = od->song->bars.range().end();
				}
				int c0 = f->read_byte();
				if ((c0 & 128) == 0) { // "running status"
					dbo("running status");
					c0 = last_status;
					f->seek(-1);
				}
				//msg_write(format("  %d %p", v, c0));
				last_status = c0;
				if (c0 == 0xff) {
					int type = f->read_byte();
					dbo(format("meta 0x%2x", type));
					int l = read_var(f);
					if (type == 0x51) {
						// set tempo
						int a0 = f->read_byte();
						int a1 = f->read_byte();
						int a2 = f->read_byte();
						mpqn = (a0 << 16) + (a1 << 8) + a2;
						dbo(format("%.1f bpm", 60000000.0f / (float)mpqn));
					} else if (type == 0x58) {
						// time signature
						int a0 = f->read_byte();
						int a1 = f->read_byte();
						int a2 = f->read_byte();
						int a3 = f->read_byte();
						numerator = a0;
						denominator = 1<<a1;
						dbo(format("time %d %d %d %d", a0, 1<<a1, a2, a3));
					} else {
						string t;
						t.resize(l);
						f->read_buffer(t.data, l);
						dbo(format("str... %d  %d  '%s'", type, l, ascii2utf8(t).hex()));
						if (type == 0x03) {
							track_name = ascii2utf8(t);
							dbo(" track name");
						} else if (type == 0x06) {
							dbo(" marker");
						} else if (type == 0x54) {
							// SMPTE offset
						} else if (type == 0x2f) {
							// end track
						}
					}
				} else if ((c0 & 0xf0) == 0xf0) {
					//dbo("sys ev");
					int l = read_var(f);
					f->seek(l);
				} else {
					int type = ((c0 >> 4) & 0x0f);
					int channel = (c0 & 0x0f);
					dbo(format("ev 0x%x  ch=%d", type, channel));
					if (type == 0x9) { // on
						int c1 = f->read_byte() & 0x7f;
						int c2 = f->read_byte() & 0x7f;
						if (!events.contains(channel))
							events.set(channel, {});
						events[channel].add(MidiEvent(offset, c1, (float)c2 / 127.0f));
						dbo(format("ON  o=%d  p=0x%02x  v=%2x   ch=%d", offset, c1, c2, channel));
					} else if (type == 0x8) { // off
						int c1 = f->read_byte() & 0x7f;
						int c2 = f->read_byte() & 0x7f;
						if (!events.contains(channel))
							events.set(channel, {});
						events[channel].add(MidiEvent(offset, c1, 0));
						dbo(format("OFF  o=%d  p=0x%02x  ch=%d", offset, c1, channel));
					} else if (type == 0x0a) { // note after touch
						f->read_byte();
						f->read_byte();
					} else if (type == 0x0b) { // controller
						int controller = f->read_byte();
						int value = f->read_byte();
						dbo(format("set controller 0x%02x=0x%02x", controller, value));
					} else if (type == 0x0c) { // program change
						int instrument = f->read_byte();
						dbo(format("set instrument 0x%02x", instrument));
					} else if (type == 0x0d) { // channel after touch
						f->read_byte();
					} else if (type == 0x0d) { // pitch bend
						f->read_byte();
						f->read_byte();
					} else {
						od->warn("unhandled midi event " + i2s(type));
					}
				}
			}

			f->set_pos(pos0 + tsize);

			if ((events.num > 0) or (i > 0)) {
				Array<int> keys = events.keys();
				for (int k : keys) {
					Track *t = od->song->add_track(SignalType::MIDI);
					t->layers[0]->midi = midi_events_to_notes(events[k]);
					t->name = track_name;
				}
			}
		}

		FileClose(f);
	} catch(Exception &e) {
		if (f)
			FileClose(f);
		od->error(e.message());
	}
}

void FormatMidi::save_song(StorageOperationData* od) {
	File *f = nullptr;
	try {
		f = FileCreate(od->filename);

		int num_tracks = 0;
		for (Track *t: weak(od->song->tracks))
			if (t->type == SignalType::MIDI)
				num_tracks ++;
		int ticks_per_beat = 96;
		// heaer
		write_chunk_name(f, "MThd");
		write_int(f, 6); // size
		f->write_word(int16_reverse(1)); // format
		f->write_word(int16_reverse(num_tracks));
		f->write_word(int16_reverse(ticks_per_beat));
		// beat = quarter note
		int mpqn = 500000; // micro s/beat = 120 beats/min;
		int numerator = 4;
		int denominator = 4;
		int last_bar = 0;
		bool first_track = true;
		for (Track* t : weak(od->song->tracks)) {
			if (t->type != SignalType::MIDI)
				continue;
			write_chunk_name(f, "MTrk");
			int pos0 = f->get_pos();
			write_int(f, 0); // size (will be replaced later)

			// track name
			if (t->name.num > 0) {
				write_var(f, 0);
				f->write_byte(0xff);
				f->write_byte(0x03);
				write_var(f, t->name.num);
				f->write_buffer(t->name);
			}
			int channel = 0;

			// instrument
			write_var(f, 0);
			f->write_byte(0xc0 + channel);
			f->write_byte(t->instrument.midi_no());

			if (od->song->bars.num > 0 and first_track) {
				auto *b = od->song->bars[0].get();
				float bpm = b->bpm(od->song->sample_rate);
				mpqn = 60000000.0f / bpm;
				write_var(f, 0);
				f->write_byte(0xff);
				f->write_byte(0x51);
				write_var(f, 3);
				f->write_byte(mpqn >> 16);
				f->write_byte(mpqn >> 8);
				f->write_byte(mpqn >> 0);

				write_var(f, 0);
				f->write_byte(0xff);
				f->write_byte(0x58);
				write_var(f, 4);
				f->write_byte(b->beats.num);
				f->write_byte(2); // 1/4
				f->write_byte(0);
				f->write_byte(0);
			}
			MidiEventBuffer events = t->layers[0]->midi.get_events(Range::ALL);
			events.sort();
			int moffset = 0;
			for (MidiEvent& e: events) {
				int v = (int) (((double) (e.pos) / (double) (mpqn) * 1000000.0
						/ (double) (od->song->sample_rate)
						* (double) (ticks_per_beat)));
				write_var(f, v - moffset);
				moffset = v;
				if (e.volume > 0) {
					// on
					f->write_byte(0x90 + channel);
					f->write_byte((int) (e.pitch));
					f->write_byte((int) (e.volume * 127.0f));
				} else {
					// off
					f->write_byte(0x80 + channel);
					f->write_byte((int) (e.pitch));
					f->write_byte(0);
				}
			}

			// end of track
			write_var(f, 0);
			f->write_byte(0xff);
			f->write_byte(0x2f);
			f->write_byte(0x00);

			int pos = f->get_pos();
			f->set_pos(pos0);
			write_int(f, pos - pos0 - 4);
			f->set_pos(pos);

			first_track = false;
		}
		FileClose(f);
	} catch(Exception &e) {
		if (f)
			FileClose(f);

		od->error(e.message());
	}


}
