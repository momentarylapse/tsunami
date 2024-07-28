/*
 * MidiSource.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "MidiSource.h"
#include "../../Session.h"
#include "../ModuleFactory.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/midi/MidiData.h"

namespace tsunami {

int MidiSource::read_midi(int port, MidiEventBuffer &midi) {
	int r = read(midi);
	return r;
}

MidiSource::MidiSource() :
	Module(ModuleCategory::MidiSource, "")
{
	produce_pos = 0;
	read_pos = 0;
	bh_midi = nullptr;




	/*bh_song = l->song();
	bh_offset = sel.range.offset;
	int b2 = bh_song->bars.getNextBeat(bh_offset);
	int b1 = bh_song->bars.getPrevBeat(b2);
	if (b1 < bh_offset)
		bh_offset = b2;
	bh_midi = &midi;*/
}

void MidiSource::__init__() {
	new(this) MidiSource;
}

void MidiSource::__delete__() {
	this->MidiSource::~MidiSource();
}

class MidiProduceData {
public:
	Bar *bar;
	int beat_no;
};

MidiProduceData create_produce_data(Song *song, int offset) {
	MidiProduceData data;
	data.bar = nullptr;
	data.beat_no = 0;
	auto bb = song->bars.get_bars(Range(offset, 0));
	if (bb.num > 0)
		data.bar = bb[0];
	return data;
}

// default: produce()...
int MidiSource::read(MidiEventBuffer &midi) {
	bh_midi = &midi;
	read_pos += midi.samples;
	while (produce_pos < read_pos) {
		auto data = create_produce_data(session->song.get(), produce_pos);
		if (!on_produce(data))
			return midi.samples;
	}
	return midi.samples;
}

void MidiSource::note(float pitch, float volume, int beats) {
	note_x(pitch, volume, beats, 0, 1);
}

void MidiSource::skip(int beats) {
	skip_x(beats, 0, 1);
}

// FIXME: argh, how can we know where to start when "applying"...
int skip_beats(int pos, Song *song, int beats, int sub_beats, int beat_partition) {
	int pos0 = pos;
	for (int i=0; i<beats; i++)
		pos = song->bars.get_next_beat(pos);
	for (int i=0; i<sub_beats; i++)
		pos = song->bars.get_next_sub_beat(pos, beat_partition);
	if (pos == pos0) {
		float dt = 60.0f / 90.0f * (beats + sub_beats / (float)beat_partition);
		pos += int(dt * song->sample_rate);
	}
	return pos;
}

void MidiSource::note_x(float pitch, float volume, int beats, int sub_beats, int beat_partition) {
	if (!bh_midi)
		return;
	int end_pos = skip_beats(produce_pos, session->song.get(), beats, sub_beats, beat_partition);
	bh_midi->add_note(Range::to(produce_pos, end_pos), pitch, volume);
}

void MidiSource::skip_x(int beats, int sub_beats, int beat_partition) {
	produce_pos = skip_beats(produce_pos, session->song.get(), beats, sub_beats, beat_partition);
}




MidiSource *CreateMidiSource(Session *session, const string &name) {
	return (MidiSource*)ModuleFactory::create(session, ModuleCategory::MidiSource, name);
}

}

