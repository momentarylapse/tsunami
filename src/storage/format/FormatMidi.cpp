/*
 * FormatMidi.cpp
 *
 *  Created on: 17.02.2013
 *      Author: michi
 */

#include "FormatMidi.h"

#include "../../lib/os/file.h"
#include "../../lib/os/formatter.h"
#include "../../lib/base/map.h"
#include "../../lib/base/iter.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/Song.h"
#include "../../data/base.h"
#include "../../data/rhythm/Bar.h"
#include "../../lib/os/msg.h"
#include "../../lib/math/math.h"

static void dbo(const string &s) {
	//msg_write(s);
}

// https://www.recordingblogs.com/wiki/midi-meta-messages

FormatDescriptorMidi::FormatDescriptorMidi() :
	FormatDescriptor("Midi", "mid,midi", Flag::MIDI | Flag::MULTITRACK | Flag::READ | Flag::WRITE)
{
}

static string read_chunk_name(BinaryFormatter *f) {
	string s;
	s.resize(4);
	*(int*)s.data = f->read_int();
	return s;
}

static void write_chunk_name(BinaryFormatter *f, const string &name) {
	string s = name;
	s.resize(4);
	f->write(s);
}

static int int_reverse(int i) {
	return ((i & 255) << 24) + (((i >> 8) & 255) << 16) + (((i >> 16) & 255) << 8) + ((i >> 24) & 255);
}

static int int16_reverse(int i) {
	return ((i & 255) << 8) + ((i >> 8) & 255);
}

static int read_int(BinaryFormatter *f) {
	int i = f->read_int();
	return int_reverse(i);
}

static void write_int(BinaryFormatter *f, int i) {
	f->write_int(int_reverse(i));
}

static int read_var(BinaryFormatter *f) {
	unsigned char c0;
	int i = 0;
	do {
		c0 = f->read_byte();
		i = (c0 & 127) + (i << 7);
	} while ((c0 & 128) > 0);
	return i;
}

static void write_var(BinaryFormatter *f, int i) {
	string s;
	int i0 = i;
	if (i < 0)
		throw Exception("VLQ < 0: " + str(i0));
	s.add(i & 0x7f);
	i >>= 7;
	if (i > 0) {
		s.add((i & 0x7f) | 0x80);
		i >>= 7;
	}
	if (i > 0) {
		s.add((i & 0x7f) | 0x80);
		i >>= 7;
	}
	if (i > 0)
		throw Exception("VLQ too large: " + str(i0));
	f->write(s.reverse());
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
	BinaryFormatter *f = nullptr;
	try {
		f = new BinaryFormatter(os::fs::open(od->filename, "rb"));

		string hn = read_chunk_name(f);
		int hsize = read_int(f);
		if (hn != "MThd")
			throw Exception(format("invalid midi header found: '%s' (expected: 'MTHd'')" + hn));
		//msg_write(hn + " " + i2s(hsize));

		int format_type = int16_reverse(f->read_word());
		dbo(format("type: %d", format_type));
		if (format_type < 0 or format_type > 2)
			throw Exception(format("invalid format type found: %d (expected 0,1,2)", format_type));
		// 0 = single track
		// 1 = multi track - 1st track only time info
		// 2 = multi track - all tracks with separate time info

		int num_tracks = int16_reverse(f->read_word());
		int time_division = int16_reverse(f->read_word());
		dbo(format("tracks: %d", num_tracks));
		dbo(format("time division: %d", time_division));

		if (hsize > 6)
			f->seek(hsize - 6);

		int ticks_per_beat = 4;

		if ((time_division & 0x8000) == 0)
			// "pulses per quarter note"
			ticks_per_beat = time_division;
		else
			// "frames per second"
			throw Exception("time division is in 'frames per second'... not implemented");

		struct TempoMarker {
			int tick_pos;
			int tick_end;
			int usec_per_beat;
			int length() const {
				return tick_end - tick_pos;
			}
		};
		Array<TempoMarker> tempo_markers = {{0, 0x7fffffff, 500000}}; // default

		struct Signature {
			int tick_pos;
			int numerator;
			int denominator;
		};
		Array<Signature> signatures = {{0, 4, 4}};

		auto ticks2samples = [od, &tempo_markers, &ticks_per_beat] (int ticks) {
			double t_sec = 0;
			int ticks_rest = ticks;
			for (auto &m: tempo_markers) {
				int dticks = min(ticks_rest, m.length());
				ticks_rest -= dticks;
				t_sec += (double)dticks * (double)m.usec_per_beat / 1000000.0 / (double)ticks_per_beat;
				if (ticks_rest <= 0)
					break;
			}
			return (int)(t_sec * (double)od->song->sample_rate);
		};

		auto create_bar_structure = [od, &signatures, &ticks_per_beat, ticks2samples] (int ticks) {
			auto find_signature_at = [&signatures] (int tick_pos) {
				Signature r;
				for (auto &s: signatures) {
					if (s.tick_pos >= tick_pos)
						break;
					r = s;
				}
				return r;
			};

			int cur_tick = 0;
			while (cur_tick < ticks) {
				auto s = find_signature_at(cur_tick + 50);
				int bar_ticks = (ticks_per_beat * s.numerator * 4) / s.denominator;
				// cheat for alignment problems...
//				if (abs(cur_tick + bar_ticks - midi_clock_offset) < 20)
//					bar_ticks = midi_clock_offset - cur_tick;

				int next_bar_end_sample = ticks2samples(cur_tick + bar_ticks);
				int bar_samples = next_bar_end_sample - od->song->bars.range().end();

				auto b = BarPattern(bar_samples, s.numerator, max(s.denominator / 4, 1));
				od->song->add_bar(-1, b, false);
				dbo("--add bar " + od->song->bars.back()->range().str());
				cur_tick += bar_ticks;
			}
		};


		od->song->add_track(SignalType::BEATS);

		int max_ticks = 0;

		for (int i=0; i<num_tracks; i++) {
			string tn = read_chunk_name(f);
			int tsize = read_int(f);
			int pos0 = f->get_pos();
			if (tn != "MTrk")
				throw Exception(format("invalid midi track header found: '%s' (expected 'MTrk')", tn));
			dbo("----------------------- track");

			base::map<int, MidiEventBuffer> events;
			string track_name;
			int last_status = 0;

			int midi_clock_offset = 0;
			while (f->get_pos() < pos0 + tsize) {
				int midi_dticks = read_var(f);
				midi_clock_offset += midi_dticks;
				//dbo(format("  <offset=%d", midi_dticks) + "   " + Range(midi_clock_offset - midi_dticks, midi_dticks).str());
				dbo(format("  <offset=%d  ->  %d", midi_dticks, midi_clock_offset));

				// samples
				int sample_pos = ticks2samples(midi_clock_offset);
				dbo(format("  <%d", sample_pos));

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
						int usec_per_beat = (a0 << 16) + (a1 << 8) + a2;
						dbo(format("set tempo %.1f bpm   (%d)", 60000000.0f / (float)usec_per_beat, usec_per_beat));
						tempo_markers.back().tick_end = midi_clock_offset;
						tempo_markers.add({midi_clock_offset, 0x7fffffff, usec_per_beat});
					} else if (type == 0x58) {
						// time signature
						int numerator = f->read_byte();
						int denominator = 1 << f->read_byte();
						int a2 = f->read_byte(); // midi clicks until metronome click
						int a3 = f->read_byte(); // 32nd notes ber beat = 8!
						dbo(format("time signature %d %d %d %d", numerator, denominator, a2, a3));
						signatures.add({midi_clock_offset, numerator, denominator});
					} else {
						string t;
						t.resize(l);
						f->read(t);
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
						events[channel].add(MidiEvent(sample_pos, c1, (float)c2 / 127.0f));
						dbo(format("ON  o=%d  p=0x%02x  v=%2x   ch=%d", sample_pos, c1, c2, channel));
					} else if (type == 0x8) { // off
						int c1 = f->read_byte() & 0x7f;
						[[maybe_unused]] int c2 = f->read_byte() & 0x7f;
						if (!events.contains(channel))
							events.set(channel, {});
						events[channel].add(MidiEvent(sample_pos, c1, 0));
						dbo(format("OFF  o=%d  p=0x%02x  ch=%d", sample_pos, c1, channel));
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
					} else if (type == 0x0e) { // pitch bend
						f->read_byte();
						f->read_byte();
					} else {
						od->warn("unhandled midi event " + i2s(type));
					}
				}
			}

			max_ticks = max(max_ticks, midi_clock_offset);

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

		create_bar_structure(max_ticks);

		delete f;
	} catch(Exception &e) {
		if (f)
			delete f;
		od->error(e.message());
	}
}

void FormatMidi::save_song(StorageOperationData* od) {
	BinaryFormatter *f = nullptr;
	try {
		f = new BinaryFormatter(os::fs::open(od->filename, "wb"));

		int num_tracks = 0;
		for (Track *t: weak(od->song->tracks))
			if (t->type == SignalType::MIDI)
				num_tracks ++;
		int ticks_per_beat = 960;
		// heaer
		write_chunk_name(f, "MThd");
		write_int(f, 6); // size
		f->write_word(int16_reverse(1)); // format
		f->write_word(int16_reverse(num_tracks));
		f->write_word(int16_reverse(ticks_per_beat));
		// beat = quarter note
		int usec_per_beat = 500000; // micro s/beat = 120 beats/min;
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
				f->write(t->name);
			}
			int channel = 0;

			// instrument
			write_var(f, 0);
			f->write_byte(0xc0 + channel);
			f->write_byte(t->instrument.midi_no());

			od->song->bars._update_offsets();

			int moffset = 0; // current message midi tick position
			int bar_midi_offset = 0;

			for (auto b: weak(od->song->bars)) {
				if (first_track) {
					float bpm = b->bpm(od->song->sample_rate);// / b->divisor;
					usec_per_beat = 60000000.0f / bpm;
					write_var(f, bar_midi_offset - moffset);
					moffset = bar_midi_offset;
					f->write_byte(0xff);
					f->write_byte(0x51);
					write_var(f, 3);
					f->write_byte(usec_per_beat >> 16);
					f->write_byte(usec_per_beat >> 8);
					f->write_byte(usec_per_beat >> 0);

					write_var(f, 0);
					f->write_byte(0xff);
					f->write_byte(0x58);
					write_var(f, 4);
					f->write_byte(b->total_sub_beats);
					f->write_byte(2); // 1/4  ...b->divisor
					f->write_byte(0);
					f->write_byte(0);

					// part markers
					for (auto m: od->song->get_parts()) {
						if (abs(m->range.offset - b->offset) < 10) {
							write_var(f, 0);
							f->write_byte(0xff);
							f->write_byte(0x01);
							write_var(f, m->text.num);
							f->write(m->text);
						}
					}
				}

				int bar_midi_ticks = b->total_sub_beats * ticks_per_beat;

				MidiEventBuffer events = t->layers[0]->midi.get_events(b->range());
				events.sort();
				for (MidiEvent& e: events) {
					// midi tick offset in bar
					int v = (int)((double)(e.pos - b->offset) / (double)b->length * (double)bar_midi_ticks);
					v = bar_midi_offset + clamp(v, 0, bar_midi_ticks);
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
				bar_midi_offset += bar_midi_ticks;
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
		delete f;
	} catch(Exception &e) {
		if (f)
			delete f;

		od->error(e.message());
	}


}
