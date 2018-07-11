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
	SignalChain(Session *session, const string &name);
	virtual ~SignalChain();

	Session *session;

	void reset();
	void save(const string &filename);
	void load(const string &filename);

	static SignalChain *create_default(Session *session);

	string name;

	Array<Module*> modules;
	Module* add(Module *m);
	Module* add(int type, const string &sub_type);
	Module* addAudioSource(const string &name);
	Module* addMidiSource(const string &name);
	Module* addAudioEffect(const string &name);
	Module* addPitchDetector();
	Module* addAudioJoiner();
	Module* addAudioSucker();
	Module* addAudioVisualizer(const string &name);
	Module* addAudioInputStream();
	Module* addAudioOutputStream();
	Module* addMidiEffect(const string &name);
	Module* addSynthesizer(const string &name);
	Module* addMidiInputStream();
	Module* addBeatMidifier();
	Module* addBeatSource(const string &name);
	void remove(Module *m);
	int module_index(Module *m);

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

	void reset_state();

	void start();
	void pause(bool paused);
	void stop();
};

#endif /* SRC_MODULE_SIGNALCHAIN_H_ */
