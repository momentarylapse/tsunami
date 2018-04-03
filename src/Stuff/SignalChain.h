/*
 * SignalChain.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_STUFF_SIGNALCHAIN_H_
#define SRC_STUFF_SIGNALCHAIN_H_

#include "Observable.h"

class AudioPort;
class MidiPort;
class BeatPort;
class ConfigPanel;
class Session;

class SignalChain : public Observable<VirtualBase>
{
public:
	SignalChain(Session *session);
	virtual ~SignalChain();

	Session *session;

	void reset();
	void save(const string &filename);
	void load(const string &filename);

	static SignalChain *create_default(Session *session);


	class Module
	{
	public:
		Module();
		virtual ~Module(){}
		float x, y;
		virtual string type() = 0;
		virtual string sub_type(){ return ""; }
		virtual string config_to_string(){ return ""; }
		virtual void config_from_string(const string &str){}
		virtual ConfigPanel *create_panel(){ return NULL; }
		virtual void set_audio_source(int port, AudioPort *s){};
		virtual void set_midi_source(int port, MidiPort *s){};
		virtual void set_beat_source(int port, BeatPort *s){};
		virtual AudioPort *audio_socket(int port){ return NULL; }
		virtual MidiPort *midi_socket(int port){ return NULL; }
		virtual BeatPort *beat_socket(int port){ return NULL; }
		Array<int> port_in, port_out;
		virtual void start(){}
		virtual void pause(bool paused){}
		virtual void stop(){}
	};
	Array<Module*> modules;
	Module* add(Module *m);
	Module* addAudioSource(const string &name);
	Module* addSongRenderer();
	Module* addMidiSource(const string &name);
	Module* addAudioEffect(const string &name);
	Module* addAudioJoiner();
	Module* addPeakMeter();
	Module* addAudioInputStream();
	Module* addAudioOutputStream();
	Module* addMidiEffect(const string &name);
	Module* addSynthesizer(const string &name);
	Module* addMidiInputStream();
	Module* addBeatMidifier();
	Module* addBeatSource(const string &name);
	void remove(Module *m);
	int module_index(SignalChain::Module *m);

	struct Cable
	{
		int type;
		Module *source, *target;
		int source_port, target_port;
	};
	Array<Cable*> cables;

	void connect(Module *source, int source_port, Module *target, int target_port);
	void disconnect_source(Module *source, int source_port);
	void disconnect_target(Module *target, int target_port);
	void disconnect(Cable *c);

	void start();
	void pause(bool paused);
	void stop();
};

#endif /* SRC_STUFF_SIGNALCHAIN_H_ */
