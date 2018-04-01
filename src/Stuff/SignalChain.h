/*
 * SignalChain.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_STUFF_SIGNALCHAIN_H_
#define SRC_STUFF_SIGNALCHAIN_H_

#include "Observable.h"

class AudioSource;
class MidiSource;
class Session;

class SignalChain : public Observable<VirtualBase>
{
public:
	SignalChain(Session *session);
	virtual ~SignalChain();

	Session *session;

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
		virtual void set_audio_source(AudioSource *s){};
		virtual void set_midi_source(MidiSource *s){};
		virtual AudioSource *audio_socket(){ return NULL; }
		virtual MidiSource *midi_socket(){ return NULL; }
		Array<int> port_in, port_out;
	};
	Array<Module*> modules;

	struct Cable
	{
		int type;
		Module *source, *target;
		int source_port, taget_port;
	};
	Array<Cable*> cables;

	void connect(Module *source, int source_port, Module *target, int target_port);
};

#endif /* SRC_STUFF_SIGNALCHAIN_H_ */
