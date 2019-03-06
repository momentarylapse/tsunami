/*
 * MidiSource.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "MidiSource.h"
#include "../../Session.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Midi/MidiData.h"


MidiSource::Output::Output(MidiSource *s) : Port(SignalType::MIDI, "out")
{
	source = s;
}

int MidiSource::Output::read_midi(MidiEventBuffer &midi)
{
	int r = source->read(midi);
	return r;
}

MidiSource::MidiSource() :
	Module(ModuleType::MIDI_SOURCE, "")
{
	port_out.add(new Output(this));

	bh_offset = 0;
	bh_midi = nullptr;




	/*bh_song = l->song();
	bh_offset = sel.range.offset;
	int b2 = bh_song->bars.getNextBeat(bh_offset);
	int b1 = bh_song->bars.getPrevBeat(b2);
	if (b1 < bh_offset)
		bh_offset = b2;
	bh_midi = &midi;*/
}

void MidiSource::__init__()
{
	new(this) MidiSource;
}

void MidiSource::__delete__()
{
	this->MidiSource::~MidiSource();
}

void MidiSource::note(float pitch, float volume, int beats)
{
	note_x(pitch, volume, beats, 0, 1);

}

void MidiSource::skip(int beats)
{
	skip_x(beats, 0, 1);
}

// FIXME: argh, how can we know where to start when "applying"...
int skip_beats(int pos, Song *song, int beats, int sub_beats, int beat_partition)
{
	int pos0 = pos;
	for (int i=0; i<beats; i++)
		pos = song->bars.get_next_beat(pos);
	for (int i=0; i<sub_beats; i++)
		pos = song->bars.get_next_sub_beat(pos, beat_partition);
	if (pos == pos0){
		float dt = 60.0f / 90.0f * (beats + sub_beats / (float)beat_partition);
		pos += int(dt * song->sample_rate);
	}
	return pos;
}

void MidiSource::note_x(float pitch, float volume, int beats, int sub_beats, int beat_partition)
{
	if (!bh_midi)
		return;
	int pos = skip_beats(bh_offset, session->song, beats, sub_beats, beat_partition);
	bh_midi->add_note(RangeTo(bh_offset, pos), pitch, volume);
	bh_offset = pos;

}

void MidiSource::skip_x(int beats, int sub_beats, int beat_partition)
{
	bh_offset = skip_beats(bh_offset, session->song, beats, sub_beats, beat_partition);
}




MidiSource *CreateMidiSource(Session *session, const string &name)
{
	return (MidiSource*)ModuleFactory::create(session, ModuleType::MIDI_SOURCE, name);
}

