/*
 * DummySynthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "DummySynthesizer.h"

#include "../../Data/Song.h"
#include "../../lib/math/math.h"


void DummySynthesizer::State::reset()
{
	for (int i=0; i<MAX_PITCH; i++){
		pitch[i].phi = 0;
		pitch[i].volume = 0;
		pitch[i].env.reset();
	}
}

DummySynthesizer::DummySynthesizer()
{
	name = "Dummy";
	state.reset();
	onConfig();
}

DummySynthesizer::~DummySynthesizer()
{
}

void DummySynthesizer::__init__()
{
	new(this) DummySynthesizer;
}


void DummySynthesizer::_set_drum(int no, float freq, float volume, float attack, float release)
{
	state.pitch[no].env.set(attack, release, 0.00001f, 0.05f, sample_rate);
	state.pitch[no].env.set2(0, volume);
	delta_phi[no] = freq * 2.0f * pi / sample_rate;
}


void DummySynthesizer::onConfig()
{
	if (instrument.type == Instrument::TYPE_DRUMS){
		for (int i=0; i<MAX_PITCH; i++){
			//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
			state.pitch[i].env.set(0.005f, 0.05f, 0.00001f, 0.05f, sample_rate);
			state.pitch[i].env.set2(0, 0.45f);
			delta_phi[i] = 100.0f * 2.0f * pi / sample_rate;
		}

		// bass drum
		_set_drum(DRUM_PITCH_BASS, 130, 1.5f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_BASS_ACCOUSTIC, 130, 1.5f, 0.005f, 0.1f);

		// snare
		_set_drum(DRUM_PITCH_SNARE, 200, 1.2f, 0.005f, 0.2f);
		_set_drum(DRUM_PITCH_SNARE_ELECTRONIC, 200, 1.2f, 0.005f, 0.2f);

		// toms
		_set_drum(DRUM_PITCH_TOM_FLOOR_LOW, 60, 0.8f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_TOM_FLOOR_HI, 100, 0.8f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_TOM_LOW, 200, 0.8f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_TOM_LOW_MID, 300, 0.6f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_TOM_HI_MID, 400, 0.5f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_TOM_HI, 600, 0.3f, 0.005f, 0.1f);

		// hihat
		_set_drum(DRUM_PITCH_HIHAT_CLOSED, 1000, 0.2f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_HIHAT_PEDAL, 1000, 0.2f, 0.005f, 0.1f);
		_set_drum(DRUM_PITCH_HIHAT_OPEN, 600, 0.15f, 0.005f, 0.4f);

		// cymbals
		_set_drum(DRUM_PITCH_CRASH_1, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DRUM_PITCH_CRASH_2, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DRUM_PITCH_RIDE_1, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DRUM_PITCH_RIDE_2, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DRUM_PITCH_CHINESE, 600, 0.23f, 0.005f, 0.4f);
		_set_drum(DRUM_PITCH_BELL_RIDE, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DRUM_PITCH_SPLASH, 600, 0.2f, 0.005f, 0.4f);

		/*DRUM_PITCH_SIDE_STICK = 37,
		DRUM_PITCH_CLAP = 39,
		DRUM_PITCH_TAMBOURINE = 54,
		DRUM_PITCH_COWBELL = 56,
		DRUM_PITCH_VIBRASLASH = 58,
		DRUM_PITCH_BONGO_HI = 60,
		DRUM_PITCH_BONGO_LOW = 61,*/
	}else{
		for (int i=0; i<MAX_PITCH; i++){
			//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
			state.pitch[i].env.set(0.005f, 0.01f, 0.5f, 0.02f, sample_rate);
			state.pitch[i].env.set2(0, 0.45f);
		}
	}
}

void DummySynthesizer::render(AudioBuffer& buf)
{
	for (int i=0; i<buf.length; i++){

		// current events?
		for (MidiEvent &e : events)
			if (e.pos == i){
				int p = e.pitch;
				State::PitchState &s = state.pitch[p];
				if (e.volume == 0){
					s.env.end();
				}else{
					s.env.start(e.volume);
					enablePitch(p, true);
				}
			}

		for (int ip=0; ip<active_pitch.num; ip++){
			int p = active_pitch[ip];
			State::PitchState &s = state.pitch[p];

			s.volume = s.env.get();

			if (s.volume == 0){
				enablePitch(p, false);
				continue;
			}

			float d = sin(s.phi) * s.volume;
			buf.c[0][i] += d;
			buf.c[1][i] += d;

			s.phi += delta_phi[p];
			if (s.phi > 8*pi)
				s.phi = loopf(s.phi, 0, 2*pi);
		}
	}
}
