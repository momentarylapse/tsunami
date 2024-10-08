/*
 * DummySynthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "DummySynthesizer.h"

#include "../../data/Song.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/math/math.h"

namespace tsunami {

class DummyPitchRenderer : public PitchRenderer {
public:
	DummyPitchRenderer(Synthesizer *synth, int pitch) : PitchRenderer(synth, pitch) {
		volume = 0;
		phi = 0;
		env = reinterpret_cast<DummySynthesizer*>(synth)->env[pitch];
	}
	bool _cdecl render(AudioBuffer &buf) override {
		for (int i=0; i<buf.length; i++){
			volume = env.get();

			if (volume == 0)
				return false;

			float d = sin(phi) * volume;
			buf.c[0][i] += d;

			phi += delta_phi;
			if (phi > 8*pi)
				phi = loop(phi, 0.0f, 2*pi);
		}
		return true;
	}
	void on_end() override {
		env.end();
	}
	void on_start(float volume) override {
		env.start(volume);
	}
	void on_config() override {
		env = ((DummySynthesizer*)synth)->env[pitch];
	}
	float volume;
	float phi;
	EnvelopeADSR env;
};


DummySynthesizer::DummySynthesizer() {
	module_class = "Dummy";
	auto_generate_stereo = true;
}

void DummySynthesizer::__init__() {
	new(this) DummySynthesizer;
}

PitchRenderer *DummySynthesizer::create_pitch_renderer(int pitch) {
	return new DummyPitchRenderer(this, pitch);
}

void DummySynthesizer::_set_drum(int no, float freq, float volume, float attack, float release) {
	env[no].set(attack, release, 0.00001f, 0.05f, sample_rate);
	env[no].set2(0, volume);
	delta_phi[no] = freq * 2.0f * pi / sample_rate;
}


void DummySynthesizer::on_config() {
	if (instrument.type == Instrument::Type::Drums) {
		for (int i=0; i<MaxPitch; i++) {
			//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
			env[i].set(0.005f, 0.05f, 0.00001f, 0.05f, sample_rate);
			env[i].set2(0, 0.45f);
			delta_phi[i] = 100.0f * 2.0f * pi / sample_rate;
		}

		// bass drum
		_set_drum(DrumPitch::Bass, 130, 1.5f, 0.005f, 0.1f);
		_set_drum(DrumPitch::BassAccoustic, 130, 1.5f, 0.005f, 0.1f);

		// snare
		_set_drum(DrumPitch::Snare, 200, 1.2f, 0.005f, 0.2f);
		_set_drum(DrumPitch::SnareElectronic, 200, 1.2f, 0.005f, 0.2f);

		// toms
		_set_drum(DrumPitch::TomFloorLow, 60, 0.8f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TomFloorHi, 100, 0.8f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TomLow, 200, 0.8f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TomLowMid, 300, 0.6f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TomHiMid, 400, 0.5f, 0.005f, 0.1f);
		_set_drum(DrumPitch::TomHi, 600, 0.3f, 0.005f, 0.1f);

		// hihat
		_set_drum(DrumPitch::HihatClosed, 1000, 0.2f, 0.005f, 0.1f);
		_set_drum(DrumPitch::HihatPedal, 1000, 0.2f, 0.005f, 0.1f);
		_set_drum(DrumPitch::HihatOpen, 600, 0.15f, 0.005f, 0.4f);

		// cymbals
		_set_drum(DrumPitch::Crash1, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::Crash2, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::Ride1, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::Ride2, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::Chinese, 600, 0.23f, 0.005f, 0.4f);
		_set_drum(DrumPitch::BellRide, 600, 0.2f, 0.005f, 0.4f);
		_set_drum(DrumPitch::Splash, 600, 0.2f, 0.005f, 0.4f);

		/*DrumPitch::SIDE_STICK = 37,
		DrumPitch::CLAP = 39,
		DrumPitch::TAMBOURINE = 54,
		DrumPitch::COWBELL = 56,
		DrumPitch::VIBRASLASH = 58,
		DrumPitch::BONGO_HI = 60,
		DrumPitch::BONGO_LOW = 61,*/
	} else {
		for (int i=0; i<MaxPitch; i++) {
			//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
			env[i].set(0.005f, 0.01f, 0.5f, 0.02f, sample_rate);
			env[i].set2(0, 0.45f);
		}
	}
}

}
