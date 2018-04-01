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
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Source/SongRenderer.h"
#include "../Audio/PeakMeter.h"

class ModuleSongRenderer : public SignalChain::Module
{
public:
	SongRenderer *renderer;
	ModuleSongRenderer(SongRenderer *r)
	{
		renderer = r;
	}
	virtual string type(){ return "SongRenderer"; }
	virtual AudioSource *get_audio_source(){ return renderer; }
};

class ModulePeakMeter : public SignalChain::Module
{
public:
	PeakMeter *peak_meter;
	ModulePeakMeter(PeakMeter *p)
	{
		peak_meter = p;
	}
	virtual string type(){ return "PeakMeter"; }
	virtual void set_audio_source(AudioSource *s){ if (peak_meter) peak_meter->set_source(s); }
};

class ModuleOutputStream : public SignalChain::Module
{
public:
	OutputStream *stream;
	ModuleOutputStream(OutputStream *s)
	{
		stream = s;
	}
	virtual string type(){ return "OutputStream"; }
	virtual void set_audio_source(AudioSource *s){ if (stream) stream->set_source(s); }
};

SignalChain::SignalChain(Session *s)
{
	session = s;
}

SignalChain::~SignalChain()
{
}

SignalChain *SignalChain::create_default(Session *session)
{
	SignalChain *chain = new SignalChain(session);

	session->song_renderer = new SongRenderer(session->song);
	session->peak_meter = new PeakMeter(session->song_renderer);
	session->output_stream = new OutputStream(session, session->peak_meter);

	chain->modules.add(new ModuleSongRenderer(session->song_renderer));
	chain->modules.add(new ModulePeakMeter(session->peak_meter));
	chain->modules.add(new ModuleOutputStream(session->output_stream));

	foreachi (Module *m, chain->modules, i){
		m->x = 50 + i * 230;
		m->y = 50;
	}

	return chain;
}

