/*
 * BeatPort.h
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_PORT_BEATPORT_H_
#define SRC_MODULE_PORT_BEATPORT_H_


#include "Port.h"
class Beat;


class BeatPort : public Port
{
public:
	BeatPort(const string &name);
	virtual ~BeatPort(){}
	virtual void _cdecl __init__(const string &name);
	virtual void _cdecl __delete__();

	virtual int _cdecl read(Array<Beat> &beats, int samples){ return 0; }
	virtual void _cdecl reset(){}
};

#endif /* SRC_MODULE_PORT_BEATPORT_H_ */
