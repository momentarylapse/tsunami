/*
 * SignalChain.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_SIGNALCHAIN_H_
#define SRC_MODULE_SIGNALCHAIN_H_

#include "../Stuff/Observable.h"

class AudioPort;
class MidiPort;
class BeatPort;
class ConfigPanel;
class Module;
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


	class _Module
	{
	public:
		_Module();
		virtual ~_Module(){}
		float x, y;
		virtual Module *configurable() = 0;
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
	Array<_Module*> modules;
	_Module* add(_Module *m);
	_Module* addAudioSource(const string &name);
	_Module* addSongRenderer();
	_Module* addMidiSource(const string &name);
	_Module* addAudioEffect(const string &name);
	_Module* addPitchDetector();
	_Module* addAudioJoiner();
	_Module* addPeakMeter();
	_Module* addAudioInputStream();
	_Module* addAudioOutputStream();
	_Module* addMidiEffect(const string &name);
	_Module* addSynthesizer(const string &name);
	_Module* addMidiInputStream();
	_Module* addBeatMidifier();
	_Module* addBeatSource(const string &name);
	void remove(_Module *m);
	int module_index(SignalChain::_Module *m);

	struct Cable
	{
		int type;
		_Module *source, *target;
		int source_port, target_port;
	};
	Array<Cable*> cables;

	void connect(_Module *source, int source_port, _Module *target, int target_port);
	void disconnect_source(_Module *source, int source_port);
	void disconnect_target(_Module *target, int target_port);
	void disconnect(Cable *c);

	void reset_state();

	void start();
	void pause(bool paused);
	void stop();
};

#endif /* SRC_MODULE_SIGNALCHAIN_H_ */
