/*
 * Synthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Synthesizer.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "DummySynthesizer.h"
#include "SampleSynthesizer.h"
#include "../../lib/math/math.h"
#include "../Port/MidiPort.h"

Synthesizer::Output::Output(Synthesizer *s) : AudioPort("out")
{
	synth = s;
}

void Synthesizer::Output::reset()
{
	synth->reset();
	if (synth->source)
		synth->source->reset();
}

int Synthesizer::Output::read(AudioBuffer &buf)
{
	if (!synth->source)
		return 0;
//	printf("synth read %d\n", buf.length);
	synth->source_run_out = false;
	// get from source...
	synth->events.samples = buf.length;
	int n = synth->source->read(synth->events);
//	printf("sr  %d", n);
	if (n == synth->source->NOT_ENOUGH_DATA){
//		printf(" no data\n");
		return NOT_ENOUGH_DATA;
	}
	if (n == synth->source->END_OF_STREAM){
		synth->source_run_out = true;

		// if source end_of_stream but still active rendering
		//  => render full requested buf size
		n = buf.length;
	}

	//printf("  %d  %d\n", synth->active_pitch.num, n);
	if (synth->has_run_out_of_data()){
//		printf(" eos\n");
		return END_OF_STREAM;
	}

//	printf("...%d  %d  ok\n", n, buf.length);
	buf.scale(0);
	synth->render(buf);

	if (synth->auto_generate_stereo)
		buf.c[1] = buf.c[0];

	synth->events.clear();

	return buf.length;
}


Synthesizer::Synthesizer() :
	Module(ModuleType::SYNTHESIZER)
{
	out = new Output(this);
	port_out.add(out);
	port_in.add(InPortDescription(SignalType::MIDI, (Port**)&source, "in"));
	sample_rate = DEFAULT_SAMPLE_RATE;
	keep_notes = 0;
	instrument = Instrument(Instrument::Type::PIANO);
	source = nullptr;
	source_run_out = false;
	auto_generate_stereo = false;

	tuning.set_default();

	set_sample_rate(DEFAULT_SAMPLE_RATE);
}

void Synthesizer::__init__()
{
	new(this) Synthesizer;
}

void Synthesizer::__delete__()
{
	this->Synthesizer::~Synthesizer();
}

void Synthesizer::Tuning::set_default()
{
	for (int p=0; p<MAX_PITCH; p++)
		freq[p] = pitch_to_freq((float)p);
}

bool Synthesizer::Tuning::is_default()
{
	for (int p=0; p<MAX_PITCH; p++)
		if (fabs(freq[p] - pitch_to_freq(p)) > 0.01f)
			return false;
	return true;
}

void Synthesizer::set_sample_rate(int _sample_rate)
{
	//if (_sample_rate == sample_rate)
	//	return;
	sample_rate = _sample_rate;

	update_delta_phi();
	on_config();
}

void Synthesizer::update_delta_phi()
{
	for (int p=0; p<MAX_PITCH; p++)
		delta_phi[p] = tuning.freq[p] * 2.0f * pi / sample_rate;
}

void Synthesizer::set_instrument(Instrument &i)
{
	instrument = i;
	on_config();
}

bool Synthesizer::has_run_out_of_data()
{
	return (active_pitch.num == 0) and source_run_out;
}

void Synthesizer::_render_part(AudioBuffer &buf, int pitch, int offset, int end)
{
	AudioBuffer part;
	part.set_as_ref(buf, offset, end - offset);
	if (!render_pitch(part, pitch))
		active_pitch.erase(pitch);
}

void Synthesizer::render(AudioBuffer& buf)
{
	Set<int> pitch_involved = active_pitch;
	for (MidiEvent &e: events)
		pitch_involved.add(e.pitch);

	for (int p: pitch_involved){
		int offset = 0;

		for (MidiEvent &e: events)
			if (e.pitch == p){
				// render before
				if (active_pitch.contains(p))
					_render_part(buf, p, offset, e.pos);

				offset = e.pos;
				handle_event(e);
				if (e.volume > 0)
					active_pitch.add(p);
			}

		if (active_pitch.contains(p))
			_render_part(buf, p, offset, buf.length);
	}

#if 0


	for (int i=0; i<buf.length; i++){

		// current events?
		for (MidiEvent &e: events)
			if (e.pos == i){
				int p = e.pitch;
				PitchState &s = pitch[p];
				if (e.volume == 0){
					s.env.end();
				}else{
					s.env.start(e.volume);
					enable_pitch(p, true);
				}
			}

		Array<int> del_me;
		for (int ip=0; ip<active_pitch.num; ip++){
			int p = active_pitch[ip];
			PitchState &s = pitch[p];

			s.volume = s.env.get();

			if (s.volume == 0)
				del_me.add(p);

			float d = sin(s.phi) * s.volume;
			buf.c[0][i] += d;

			s.phi += delta_phi[p];
			if (s.phi > 8*pi)
				s.phi = loopf(s.phi, 0, 2*pi);
		}

		for (int p: del_me)
			enable_pitch(p, false);

		if (buf.channels > 1)
			buf.c[1][i] = buf.c[0][i];
	}
#endif
}

void Synthesizer::reset()
{
	reset_state();
	active_pitch.clear();
	source_run_out = false;
}

bool Synthesizer::is_default()
{
	return (module_subtype == "Dummy") and (tuning.is_default());
}

Synthesizer* CreateSynthesizer(Session *session, const string &name)
{
	return (Synthesizer*)ModuleFactory::create(session, ModuleType::SYNTHESIZER, name);
}

