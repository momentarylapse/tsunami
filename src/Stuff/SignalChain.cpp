/*
 * SignalChain.cpp
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#include "SignalChain.h"

#include "../Session.h"
#include "../Device/OutputStream.h"
#include "../Device/InputStreamAudio.h"
#include "../Device/InputStreamMidi.h"
#include "../Plugins/Effect.h"
#include "../Plugins/MidiEffect.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Source/SongRenderer.h"
#include "../Audio/PeakMeter.h"

SignalChain::Module::Module()
{
	x = y = 0;
}

class ModuleSongRenderer : public SignalChain::Module
{
public:
	SongRenderer *renderer;
	ModuleSongRenderer(SongRenderer *r)
	{
		renderer = r;
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleSongRenderer(){ delete renderer; }
	virtual string type(){ return "SongRenderer"; }
	virtual AudioSource *audio_socket(){ return renderer; }
};

class ModulePeakMeter : public SignalChain::Module
{
public:
	PeakMeter *peak_meter;
	ModulePeakMeter(PeakMeter *p)
	{
		peak_meter = p;
		port_in.add(Track::Type::AUDIO);
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModulePeakMeter(){ delete peak_meter; }
	virtual string type(){ return "PeakMeter"; }
	virtual void set_audio_source(AudioSource *s){ if (peak_meter) peak_meter->set_source(s); }
	virtual AudioSource *audio_socket(){ return peak_meter; }
};

class ModuleOutputStream : public SignalChain::Module
{
public:
	OutputStream *stream;
	ModuleOutputStream(OutputStream *s)
	{
		stream = s;
		port_in.add(Track::Type::AUDIO);
	}
	virtual ~ModuleOutputStream(){ delete stream; }
	virtual string type(){ return "OutputStream"; }
	virtual void set_audio_source(AudioSource *s){ if (stream) stream->set_source(s); }
};

class ModuleAudioEffect : public SignalChain::Module
{
public:
	Effect *fx;
	ModuleAudioEffect(Effect *_fx)
	{
		fx = _fx;
		port_in.add(Track::Type::AUDIO);
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleAudioEffect(){ delete fx; }
	virtual string type(){ return "AudioEffect"; }
	virtual AudioSource *audio_socket(){ return fx->out; }
	virtual void set_audio_source(AudioSource *s){ if (fx) fx->out->set_source(s); }
};

class ModuleMidiEffect : public SignalChain::Module
{
public:
	MidiEffect *fx;
	ModuleMidiEffect(MidiEffect *_fx)
	{
		fx = _fx;
		port_in.add(Track::Type::MIDI);
		port_out.add(Track::Type::MIDI);
	}
	virtual ~ModuleMidiEffect(){ delete fx; }
	virtual string type(){ return "MidiEffect"; }
	//virtual void set_midi_source(MidiSource *s){ if (fx) fx->out->set_source(s); }
	//virtual MidiSource *midi_socket(){ return fx->out; }
};

class ModuleSynthesizer : public SignalChain::Module
{
public:
	Synthesizer *synth;
	ModuleSynthesizer(Synthesizer *s)
	{
		synth = s;
		port_in.add(Track::Type::MIDI);
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleSynthesizer(){ delete synth; }
	virtual string type(){ return "Synthesizer"; }
	virtual AudioSource *audio_socket(){ return synth->out; }
	virtual void set_midi_source(MidiSource *s){ if (synth) synth->out->set_source(s); }
};

SignalChain::SignalChain(Session *s)
{
	session = s;
}

SignalChain::~SignalChain()
{
	for (Module *m: modules)
		delete(m);
}

SignalChain *SignalChain::create_default(Session *session)
{
	SignalChain *chain = new SignalChain(session);

	session->song_renderer = new SongRenderer(session->song);
	Effect *fx = CreateEffect(session, "Echo");
	fx->configFromString("(0.3 0.98 0.5)");
	session->peak_meter = new PeakMeter(NULL);
	session->output_stream = new OutputStream(session, NULL);

	auto *mod_renderer = new ModuleSongRenderer(session->song_renderer);
	auto *mod_fx = new ModuleAudioEffect(fx);
	auto *mod_peak = new ModulePeakMeter(session->peak_meter);
	auto *mod_out = new ModuleOutputStream(session->output_stream);
	chain->modules.add(mod_renderer);
	chain->modules.add(mod_fx);
	chain->modules.add(mod_peak);
	chain->modules.add(mod_out);

	chain->connect(mod_renderer, 0, mod_fx, 0);
	chain->connect(mod_fx, 0, mod_peak, 0);
	chain->connect(mod_peak, 0, mod_out, 0);

	foreachi (Module *m, chain->modules, i){
		m->x = 50 + i * 230;
		m->y = 50;
	}

	return chain;
}

void SignalChain::connect(SignalChain::Module *source, int source_port, SignalChain::Module *target, int target_port)
{
	if (source_port < 0 or source_port >= source->port_out.num)
		throw Exception("bla");
	if (target_port < 0 or target_port >= target->port_in.num)
		throw Exception("bla");
	if (source->port_out[source_port] != target->port_in[target_port])
		throw Exception("bla");
	// TODO: check ports in use
	Cable *c = new Cable;
	c->type = source->port_out[source_port];
	c->source = source;
	c->target = target;
	c->source_port = source_port;
	c->taget_port = target_port;
	cables.add(c);
	if (c->type == Track::Type::AUDIO){
		target->set_audio_source(source->audio_socket());
	}else if (c->type == Track::Type::MIDI){
		target->set_midi_source(source->midi_socket());
	}
}


