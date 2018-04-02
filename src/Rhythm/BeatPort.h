/*
 * BeatPort.h
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#ifndef SRC_RHYTHM_BEATPORT_H_
#define SRC_RHYTHM_BEATPORT_H_


#include "../lib/base/base.h"
class Beat;


class BeatPort : public VirtualBase
{
public:
	virtual ~BeatPort(){}
	virtual void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int _cdecl read(Array<Beat> &beats, int samples){ return 0; }
	virtual void _cdecl reset(){}
};

#endif /* SRC_RHYTHM_BEATPORT_H_ */
