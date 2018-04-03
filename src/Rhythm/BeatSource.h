/*
 * BeatSource.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_RHYTHM_BEATSOURCE_H_
#define SRC_RHYTHM_BEATSOURCE_H_

#include "BeatPort.h"
#include "../Module/Module.h"

class DummyBeatSource;

class BeatSource : public Module
{
public:
	BeatSource();
	virtual ~BeatSource();

	void _cdecl __init__();
	void _cdecl __delete__();

	class Output : public BeatPort
	{
	public:
		Output(BeatSource *s);
		virtual ~Output(){}
		virtual int _cdecl read(Array<Beat> &beats, int samples);
		virtual void _cdecl reset();
		BeatSource *source;
	};
	Output *out;

	virtual int _cdecl read(Array<Beat> &beats, int samples){ return samples; }
	virtual void _cdecl reset(){}

	static DummyBeatSource *dummy;
};

class DummyBeatSource : public BeatSource
{
public:
	virtual int _cdecl read(Array<Beat> &beats, int samples){ return samples; }
};

BeatSource *CreateBeatSource(Session *session, const string &name);


#endif /* SRC_RHYTHM_BEATSOURCE_H_ */
