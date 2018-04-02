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
#include "../Plugins/MidiEffect.h"
#include "../Plugins/PluginManager.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Source/SongRenderer.h"
#include "../Audio/PeakMeter.h"
#include "../Audio/AudioJoiner.h"
#include "../Midi/MidiSource.h"
#include "../Midi/MidiEventStreamer.h"
#include "../Rhythm/BeatSource.h"
#include "../Rhythm/BarStreamer.h"
#include "../Rhythm/BarCollection.h"
#include "../Rhythm/Bar.h"
#include "../lib/file/file.h"
#include "../Plugins/AudioEffect.h"

SignalChain::Module::Module()
{
	x = y = 0;
}

class ModuleAudioSource : public SignalChain::Module
{
public:
	AudioPort *source;
	ModuleAudioSource(AudioPort *s)
	{
		source = s;
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleAudioSource(){ delete source; }
	virtual string type(){ return "AudioSource"; }
	virtual AudioPort *audio_socket(int port){ return source; }
};

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
	virtual AudioPort *audio_socket(int port){ return renderer; }
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
	virtual void set_audio_source(int port, AudioPort *s){ peak_meter->set_source(s); }
	virtual AudioPort *audio_socket(int port){ return peak_meter; }
};

class ModuleAudioJoiner : public SignalChain::Module
{
public:
	AudioJoiner *joiner;
	ModuleAudioJoiner(AudioJoiner *j)
	{
		joiner = j;
		port_in.add(Track::Type::AUDIO);
		port_in.add(Track::Type::AUDIO);
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleAudioJoiner(){ delete joiner; }
	virtual string type(){ return "AudioJoiner"; }
	virtual void set_audio_source(int port, AudioPort *s)
	{
		if (port == 0)
			joiner->set_source_a(s);
		else if (port == 1)
			joiner->set_source_b(s);
	}
	virtual AudioPort *audio_socket(int port){ return joiner; }
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
	virtual void set_audio_source(int port, AudioPort *s){ stream->set_source(s); }
};

class ModuleAudioInputStream : public SignalChain::Module
{
public:
	InputStreamAudio *stream;
	ModuleAudioInputStream(InputStreamAudio *s)
	{
		stream = s;
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleAudioInputStream(){ delete stream; }
	virtual string type(){ return "AudioInputStream"; }
	virtual AudioPort *audio_socket(int port){ return stream->out; }
};

class ModuleAudioEffect : public SignalChain::Module
{
public:
	AudioEffect *fx;
	ModuleAudioEffect(AudioEffect *_fx)
	{
		fx = _fx;
		port_in.add(Track::Type::AUDIO);
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleAudioEffect(){ delete fx; }
	virtual string type(){ return "AudioEffect"; }
	virtual AudioPort *audio_socket(int port){ return fx->out; }
	virtual void set_audio_source(int port, AudioPort *s){ fx->set_source(s); }

	virtual string sub_type(){ return fx->name; }
	virtual string config_to_string(){ return fx->config_to_string(); }
	virtual void config_from_string(const string &str){ fx->config_from_string(str); }
	virtual ConfigPanel *create_panel(){ return fx->create_panel(); }
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
	//virtual void set_midi_source(int port, MidiSource *s){ if (fx) fx->out->set_source(s); }
	//virtual MidiSource *midi_socket(int port){ return fx->out; }
	virtual string sub_type(){ return fx->name; }
	virtual string config_to_string(){ return fx->config_to_string(); }
	virtual void config_from_string(const string &str){ fx->config_from_string(str); }
	virtual ConfigPanel *create_panel(){ return fx->create_panel(); }
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
	virtual AudioPort *audio_socket(int port){ return synth->out; }
	virtual void set_midi_source(int port, MidiSource *s){ synth->set_source(s); }

	virtual string sub_type(){ return synth->name; }
	virtual string config_to_string(){ return synth->config_to_string(); }
	virtual void config_from_string(const string &str){ synth->config_from_string(str); }
	virtual ConfigPanel *create_panel(){ return synth->create_panel(); }
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
	virtual MidiSource *midi_socket(int port){ return beat_midifier; }
	virtual void set_beat_source(int port, BeatSource *s){ beat_midifier->setBeatSource(s); }
};

class ModuleBeatSource : public SignalChain::Module
{
public:
	BeatSource *source;
	ModuleBeatSource(BeatSource *s)
	{
		source = s;
		port_out.add(Track::Type::TIME);
	}
	virtual ~ModuleBeatSource(){ delete source; }
	virtual string type(){ return "BeatSource"; }
	virtual BeatSource *beat_socket(int port){ return source; }
};

class ModuleMidiSource: public SignalChain::Module
{
public:
	MidiSource *source;
	ModuleMidiSource(MidiSource *s)
	{
		source = s;
		port_out.add(Track::Type::MIDI);
	}
	virtual ~ModuleMidiSource(){ delete source; }
	virtual string type(){ return "MidiSource"; }
	virtual MidiSource *midi_socket(int port){ return source; }
};

class ModuleMidiInputStream : public SignalChain::Module
{
public:
	InputStreamMidi *stream;
	ModuleMidiInputStream(InputStreamMidi *s)
	{
		stream = s;
		port_out.add(Track::Type::AUDIO);
	}
	virtual ~ModuleMidiInputStream(){ delete stream; }
	virtual string type(){ return "MidiInputStream"; }
	virtual MidiSource *midi_socket(int port){ return stream->out; }
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

	auto *mod_renderer = chain->addSongRenderer();
	auto *mod_peak = chain->addPeakMeter();
	auto *mod_out = chain->addAudioOutputStream();

	session->song_renderer = dynamic_cast<ModuleSongRenderer*>(mod_renderer)->renderer;
	session->peak_meter = dynamic_cast<ModulePeakMeter*>(mod_peak)->peak_meter;
	session->output_stream = dynamic_cast<ModuleOutputStream*>(mod_out)->stream;

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
		target->set_audio_source(target_port, source->audio_socket(source_port));
	}else if (c->type == Track::Type::MIDI){
		target->set_midi_source(target_port, source->midi_socket(source_port));
	}else if (c->type == Track::Type::TIME){
		target->set_beat_source(target_port, source->beat_socket(source_port));
	}
	notify();
}

void SignalChain::disconnect(SignalChain::Cable *c)
{
	foreachi(Cable *cc, cables, i)
		if (cc == c){
			if (c->type == Track::Type::AUDIO){
				c->target->set_audio_source(c->target_port, NULL);
			}else if (c->type == Track::Type::MIDI){
				c->target->set_midi_source(c->target_port, NULL);
			}else if (c->type == Track::Type::TIME){
				c->target->set_beat_source(c->target_port, NULL);
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

void SignalChain::save(const string& filename)
{
	File* f = FileCreateText(filename);
	f->write_str("1");
	f->write_int(modules.num);
	for (auto *m: modules){
		f->write_str(m->type());
		f->write_str(m->sub_type());
		f->write_str("");
		f->write_str(m->config_to_string());
		f->write_int(m->x);
		f->write_int(m->y);
	}
	f->write_int(cables.num);
	for (auto *c: cables){
		f->write_int(module_index(c->source));
		f->write_int(c->source_port);
		f->write_int(module_index(c->target));
		f->write_int(c->target_port);
	}
	delete f;
}

void SignalChain::load(const string& filename)
{
	for (int i=modules.num-1; i>=3; i--)
		remove(modules[i]);
	Array<Cable*> _cables = cables;
	for (auto *c: cables)
		disconnect(c);

	File *f = FileOpenText(filename);
	f->read_str();
	int n = f->read_int();
	for (int i=0; i<n; i++){
		string type = f->read_str();
		string sub_type = f->read_str();
		string name = f->read_str();
		Module *m = NULL;
		if (i < 3){
			m = modules[i];
		}else{
			if (type == "AudioSource")
				m = addAudioSource(sub_type);
			else if (type == "AudioEffect")
				m = addAudioEffect(sub_type);
			else if (type == "AudioJoiner")
				m = addAudioJoiner();
			else if (type == "PeakMeter")
				m = addPeakMeter();
			else if (type == "AudioInputStream")
				m = addAudioInputStream();
			else if (type == "OutputStream")
				m = addAudioOutputStream();
			else if (type == "MidiSource")
				m = addMidiSource(sub_type);
			else if (type == "MidiEffect")
				m = addMidiEffect(sub_type);
			else if (type == "Synthesizer")
				m = addSynthesizer(sub_type);
			else if (type == "MidiInputStream")
				m = addMidiInputStream();
			else if (type == "BeatMidifier")
				m = addBeatMidifier();
			else if (type == "BeatSource")
				m = addBeatSource(sub_type);
			else
				throw Exception("unhandled module type");
		}
		string config = f->read_str();
		m->config_from_string(config);
		m->x = f->read_int();
		m->y = f->read_int();
	}
	n = f->read_int();
	for (int i=0; i<n; i++){
		int s = f->read_int();
		int sp = f->read_int();
		int t = f->read_int();
		int tp = f->read_int();
		connect(modules[s], sp, modules[t], tp);
	}
	delete f;
}

void SignalChain::reset()
{
	for (int i=modules.num-1; i>=3; i--)
		remove(modules[i]);
	Array<Cable*> _cables = cables;
	for (auto *c: cables)
		disconnect(c);

	connect(modules[0], 0, modules[1], 0);
	connect(modules[1], 0, modules[2], 0);
}

SignalChain::Module* SignalChain::addSongRenderer()
{
	return add(new ModuleSongRenderer(new SongRenderer(session->song)));
}

SignalChain::Module* SignalChain::addAudioSource(const string &name)
{
	return NULL;
	//return add(new ModuleAudioSource(s));
}

SignalChain::Module* SignalChain::addMidiSource(const string &name)
{
	MidiEventBuffer midi;
	midi.add(MidiEvent(100, 80, 1));
	midi.add(MidiEvent(40000, 80, 0));
	midi.add(MidiEvent(50000, 82, 1));
	midi.add(MidiEvent(90000, 82, 0));
	midi.samples = 400000;
	return add(new ModuleMidiSource(new MidiEventStreamer(midi)));
}

SignalChain::Module* SignalChain::addAudioEffect(const string &name)
{
	return add(new ModuleAudioEffect(CreateAudioEffect(session, name)));
}

SignalChain::Module* SignalChain::addAudioJoiner()
{
	return add(new ModuleAudioJoiner(new AudioJoiner(NULL, NULL)));
}

SignalChain::Module* SignalChain::addPeakMeter()
{
	return add(new ModulePeakMeter(new PeakMeter(NULL)));
}

SignalChain::Module* SignalChain::addAudioInputStream()
{
	return add(new ModuleAudioInputStream(new InputStreamAudio(session, DEFAULT_SAMPLE_RATE)));
}

SignalChain::Module* SignalChain::addAudioOutputStream()
{
	return add(new ModuleOutputStream(new OutputStream(session, NULL)));
}

SignalChain::Module* SignalChain::addMidiEffect(const string &name)
{
	return add(new ModuleMidiEffect(CreateMidiEffect(session, name)));
}

SignalChain::Module* SignalChain::addSynthesizer(const string &name)
{
	return add(new ModuleSynthesizer(session->plugin_manager->CreateSynthesizer(session, name)));
}

SignalChain::Module* SignalChain::addMidiInputStream()
{
	return add(new ModuleMidiInputStream(new InputStreamMidi(session, DEFAULT_SAMPLE_RATE)));
}

SignalChain::Module* SignalChain::addBeatMidifier()
{
	return add(new ModuleBeatMidifier(new BeatMidifier));
}

SignalChain::Module* SignalChain::addBeatSource(const string &name)
{
	BarCollection bars;
	bars.add(new Bar(80000, 4, 1));
	bars.add(new Bar(80000, 4, 1));
	bars.add(new Bar(80000, 4, 1));
	bars.add(new Bar(80000, 4, 1));
	return add(new ModuleBeatSource(new BarStreamer(bars)));
}

