/*
 * BeatSource.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_RHYTHM_BEATSOURCE_H_
#define SRC_RHYTHM_BEATSOURCE_H_

#include "../lib/base/base.h"
class Beat;


class DummyBeatSource;

class BeatSource : public VirtualBase
{
public:
	virtual ~BeatSource(){}
	virtual int _cdecl read(Array<Beat> &beats, int samples) = 0;

	static DummyBeatSource *dummy;
};

class DummyBeatSource : public BeatSource
{
public:
	virtual int _cdecl read(Array<Beat> &beats, int samples){ return samples; }
};



#endif /* SRC_RHYTHM_BEATSOURCE_H_ */
