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
		_set_drum(35, 130, 0.8f, 0.005f, 0.1f);
		_set_drum(36, 130, 0.8f, 0.005f, 0.1f);

		// snare
		_set_drum(38, 200, 0.8f, 0.005f, 0.2f);

		// toms
		_set_drum(41, 60, 0.8f, 0.005f, 0.1f);
		_set_drum(43, 100, 0.8f, 0.005f, 0.1f);
		_set_drum(45, 200, 0.8f, 0.005f, 0.1f);
		_set_drum(47, 300, 0.8f, 0.005f, 0.1f);
		_set_drum(48, 400, 0.8f, 0.005f, 0.1f);
		_set_drum(50, 600, 0.8f, 0.005f, 0.1f);

		// hihat
		_set_drum(42, 1000, 0.4f, 0.005f, 0.1f); // closed
		_set_drum(46, 600, 0.3f, 0.005f, 0.4f); // open

		// cymbals
		_set_drum(49, 600, 0.3f, 0.005f, 0.4f);
		_set_drum(51, 600, 0.3f, 0.005f, 0.4f);
		_set_drum(52, 600, 0.3f, 0.005f, 0.4f);
		_set_drum(53, 600, 0.3f, 0.005f, 0.4f);
		_set_drum(55, 600, 0.3f, 0.005f, 0.4f);
		_set_drum(57, 600, 0.3f, 0.005f, 0.4f);
	}else{
		for (int i=0; i<MAX_PITCH; i++){
			//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
			state.pitch[i].env.set(0.005f, 0.01f, 0.5f, 0.02f, sample_rate);
			state.pitch[i].env.set2(0, 0.45f);
		}
	}
}

void DummySynthesizer::render(BufferBox& buf)
{
	for (int i=0; i<buf.num; i++){

		// current events?
		foreach(MidiEvent &e, events)
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
			buf.r[i] += d;
			buf.l[i] += d;

			s.phi += delta_phi[p];
			if (s.phi > 8*pi)
				s.phi = loopf(s.phi, 0, 2*pi);
		}
	}
}
