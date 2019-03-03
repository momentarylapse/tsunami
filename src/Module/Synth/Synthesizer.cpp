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

PitchRenderer::PitchRenderer(Synthesizer *s, int p)
{
	synth = s;
	pitch = p;
	delta_phi = synth->delta_phi[pitch];
}

void PitchRenderer::__init__(Synthesizer *s, int p)
{
	new(this) PitchRenderer(s, p);
}

void PitchRenderer::__delete__()
{
	this->PitchRenderer::~PitchRenderer();
}

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

PitchRenderer *Synthesizer::create_pitch_renderer(int pitch)
{
	return nullptr;
}

PitchRenderer *Synthesizer::get_pitch_renderer(int pitch)
{
	for (auto *p: pitch_renderer)
		if (p->pitch == pitch){
			return p;
		}
	auto *p = create_pitch_renderer(pitch);
	if (p){
		p->on_config();
		pitch_renderer.add(p);
	}
	return p;
}

void Synthesizer::_render_part(AudioBuffer &buf, int pitch, int offset, int end)
{
	if (offset == end)
		return;

	AudioBuffer part;
	part.set_as_ref(buf, offset, end - offset);
	PitchRenderer *pr = get_pitch_renderer(pitch);
	if (!pr)
		return;
	if (!pr->render(part))
		active_pitch.erase(pitch);
}

void Synthesizer::_handle_event(const MidiEvent &e)
{
	PitchRenderer *pr = get_pitch_renderer(e.pitch);
	if (!pr)
		return;
	if (e.volume > 0){
		pr->on_start(e.volume);
	}else{
		pr->on_end();
	}
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
				_handle_event(e);
				if (e.volume > 0)
					active_pitch.add(p);
			}

		if (active_pitch.contains(p))
			_render_part(buf, p, offset, buf.length);
	}
}

void Synthesizer::on_config()
{
	for (auto *p: pitch_renderer)
		p->on_config();
}

void Synthesizer::reset()
{
	reset_state();
	for (auto *p: pitch_renderer)
		delete p;
	pitch_renderer.clear();
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

