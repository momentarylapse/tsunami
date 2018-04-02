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
#include "../Plugins/PluginManager.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Source/SongRenderer.h"
#include "../Audio/PeakMeter.h"
#include "../Midi/MidiSource.h"
#include "../Midi/MidiEventStreamer.h"
#include "../Rhythm/BarStreamer.h"
#include "../Rhythm/BarCollection.h"
#include "../Rhythm/Bar.h"

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

class ModuleBeatMidifier : public SignalChain::Module
{
public:
	BeatMidifier *beat_midifier;
	ModuleBeatMidifier(BeatMidifier *b)
	{
		beat_midifier = b;
		port_in.add(Track::Type::TIME);
		port_out.add(Track::Type::MIDI);
	}
	virtual ~ModuleBeatMidifier(){ delete beat_midifier; }
	virtual string type(){ return "BeatMidifier"; }
	virtual MidiSource *midi_socket(){ return beat_midifier; }
	virtual void set_beat_source(BeatSource *s){ if (beat_midifier) beat_midifier->setBeatSource(s); }
};

class ModuleBarStreamer : public SignalChain::Module
{
public:
	BarStreamer *bar_streamer;
	ModuleBarStreamer(BarStreamer *b)
	{
		bar_streamer = b;
		port_out.add(Track::Type::TIME);
	}
	virtual ~ModuleBarStreamer(){ delete bar_streamer; }
	virtual string type(){ return "BarStreamer"; }
	virtual BeatSource *beat_socket(){ return bar_streamer; }
};

class ModuleMidiStreamer : public SignalChain::Module
{
public:
	MidiEventStreamer *midi_streamer;
	ModuleMidiStreamer(MidiEventStreamer *s)
	{
		midi_streamer = s;
		port_out.add(Track::Type::MIDI);
	}
	virtual ~ModuleMidiStreamer(){ delete midi_streamer; }
	virtual string type(){ return "MidiStreamer"; }
	virtual MidiSource *midi_socket(){ return midi_streamer; }
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

	auto *mod_renderer = chain->add(new ModuleSongRenderer(session->song_renderer));
	auto *mod_peak = chain->add(new ModulePeakMeter(session->peak_meter));
	auto *mod_out = chain->add(new ModuleOutputStream(session->output_stream));
	chain->add(new ModuleAudioEffect(fx));
	chain->add(new ModuleSynthesizer(session->plugin_manager->CreateSynthesizer(session, "")));
	BarCollection bars;
	bars.add(new Bar(80000, 4, 1));
	bars.add(new Bar(80000, 4, 1));
	bars.add(new Bar(80000, 4, 1));
	bars.add(new Bar(80000, 4, 1));
	chain->add(new ModuleBarStreamer(new BarStreamer(bars)));
	chain->add(new ModuleBeatMidifier(new BeatMidifier));
	MidiEventBuffer midi;
	midi.add(MidiEvent(100, 80, 1));
	midi.add(MidiEvent(40000, 80, 0));
	midi.add(MidiEvent(50000, 82, 1));
	midi.add(MidiEvent(90000, 82, 0));
	midi.samples = 400000;
	chain->add(new ModuleMidiStreamer(new MidiEventStreamer(midi)));

	//chain->connect(mod_renderer, 0, mod_fx, 0);
	//chain->connect(mod_fx, 0, mod_peak, 0);
	chain->connect(mod_renderer, 0, mod_peak, 0);
	chain->connect(mod_peak, 0, mod_out, 0);

	return chain;
}

SignalChain::Module *SignalChain::add(SignalChain::Module *m)
{
	int i = modules.num;
	m->x = 50 + (i % 5) * 230;
	m->y = 50 + (i % 2) * 30 + 150*(i / 5);

	modules.add(m);
	return m;
}

int SignalChain::module_index(SignalChain::Module *m)
{
	foreachi(Module *mm, modules, i)
		if (mm == m)
			return i;
	return -1;
}

void SignalChain::remove(SignalChain::Module *m)
{
	int index = module_index(m);
	if (index < 0)
		return;
	if (index < 3){
		session->e(_("not allowed to delete system modules"));
		return;
	}


	bool more = true;
	while (more){
		more = false;
		for (Cable *c: cables)
			if (c->source == m or c->target == m){
				disconnect(c);
				more = true;
				break;
			}
	}

	modules.erase(index);
	delete m;
	notify();
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
	c->target_port = target_port;
	cables.add(c);
	if (c->type == Track::Type::AUDIO){
		target->set_audio_source(source->audio_socket());
	}else if (c->type == Track::Type::MIDI){
		target->set_midi_source(source->midi_socket());
	}else if (c->type == Track::Type::TIME){
		target->set_beat_source(source->beat_socket());
	}
	notify();
}

void SignalChain::disconnect(SignalChain::Cable *c)
{
	foreachi(Cable *cc, cables, i)
		if (cc == c){
			if (c->type == Track::Type::AUDIO){
				c->target->set_audio_source(NULL);
			}else if (c->type == Track::Type::MIDI){
				c->target->set_midi_source(NULL);
			}else if (c->type == Track::Type::TIME){
				c->target->set_beat_source(NULL);
			}

			delete(c);
			cables.erase(i);
			notify();
			break;
		}
}

void SignalChain::disconnect_source(SignalChain::Module *source, int source_port)
{
	for (Cable *c: cables)
		if (c->source == source and c->source_port == source_port){
			disconnect(c);
			break;
		}
}

void SignalChain::disconnect_target(SignalChain::Module *target, int target_port)
{
	for (Cable *c: cables)
		if (c->target == target and c->target_port == target_port){
			disconnect(c);
			break;
		}
}

