/*
 * DummySynthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "DummySynthesizer.h"

#include "../../Data/Song.h"
#include "../../lib/math/math.h"


void DummySynthesizer::reset_state()
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
	reset_state();
	on_config();
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
	pitch[no].env.set(attack, release, 0.00001f, 0.05f, sample_rate);
	pitch[no].env.set2(0, volume);
	delta_phi[no] = freq * 2.0f * pi / sample_rate;
}


void DummySynthesizer::on_config()
{
	if (instrument.type == Instrument::Type::DRUMS){
		for (int i=0; i<MAX_PITCH; i++){
			//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
			pitch[i].env.set(0.005f, 0.05f, 0.00001f, 0.05f, sample_rate);
			pitch[i].env.set2(0, 0.45f);
			delta_phi[i] = 100.0f * 2.0f * pi / sample_rate;
		}

		// bass drum
		_set_drum(DrumPitch::BASS, 130, 1.5f, 0.005f, 0.1f);
		_set_drum(DrumPitch::BASS_ACCOUSTIC, 130, 1.5f, 0.005f, 0.1f);

		// snare
		_set_drum(DrumPitch::SNARE, 200, 1.2f, 0.005f, 0.2f);
		_set_drum(DrumPitch::SNARE_ELECTRONIC, 200, 1.2f, 0.005f, 0.2f);

		// toms
		_set_drum(DrumPitch::TOM_FLOOR_LOW, 60, 0.8f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TOM_FLOOR_HI, 100, 0.8f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TOM_LOW, 200, 0.8f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TOM_LOW_MID, 300, 0.6f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TOM_HI_MID, 400, 0.5f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TOM_HI, 600, 0.3f, 0.005f, 0.1f);

		// hihat
		_set_drum(DrumPitch::HIHAT_CLOSED, 1000, 0.2f, 0.005f, 0.1f);
		_set_drum(DrumPitch::HIHAT_PEDAL, 1000, 0.2f, 0.005f, 0.1f);
		_set_drum(DrumPitch::HIHAT_OPEN, 600, 0.15f, 0.005f, 0.4f);

		// cymbals
		_set_drum(DrumPitch::CRASH_1, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::CRASH_2, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::RIDE_1, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::RIDE_2, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::CHINESE, 600, 0.23f, 0.005f, 0.4f);
		_set_drum(DrumPitch::BELL_RIDE, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::SPLASH, 600, 0.2f, 0.005f, 0.4f);

		/*DrumPitch::SIDE_STICK = 37,
		DrumPitch::CLAP = 39,
		DrumPitch::TAMBOURINE = 54,
		DrumPitch::COWBELL = 56,
		DrumPitch::VIBRASLASH = 58,
		DrumPitch::BONGO_HI = 60,
		DrumPitch::BONGO_LOW = 61,*/
	}else{
		for (int i=0; i<MAX_PITCH; i++){
			//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
			pitch[i].env.set(0.005f, 0.01f, 0.5f, 0.02f, sample_rate);
			pitch[i].env.set2(0, 0.45f);
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
				PitchState &s = pitch[p];
				if (e.volume == 0){
					s.env.end();
				}else{
					s.env.start(e.volume);
					enablePitch(p, true);
				}
			}

		for (int ip=0; ip<active_pitch.num; ip++){
			int p = active_pitch[ip];
			PitchState &s = pitch[p];

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
