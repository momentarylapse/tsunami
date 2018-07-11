/*
 * BeatSource.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_BEATS_BEATSOURCE_H_
#define SRC_MODULE_BEATS_BEATSOURCE_H_

#include "../Port/BeatPort.h"
#include "../Module.h"

class DummyBeatSource;

class BeatSource : public Module
{
public:
	BeatSource();
	virtual ~BeatSource();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	class Output : public BeatPort
	{
	public:
		Output(BeatSource *s);
		virtual ~Output() override {}
		int _cdecl read(Array<Beat> &beats, int samples) override;
		void _cdecl reset() override;
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
	int _cdecl read(Array<Beat> &beats, int samples) override { return samples; }
};

BeatSource *CreateBeatSource(Session *session, const string &name);


#endif /* SRC_MODULE_BEATS_BEATSOURCE_H_ */
