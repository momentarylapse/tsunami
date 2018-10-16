/*
 * AudioPort.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_MODULE_PORT_AUDIOPORT_H_
#define SRC_MODULE_PORT_AUDIOPORT_H_

#include "Port.h"

class AudioBuffer;

class AudioPort : public Port
{
public:
	AudioPort(const string &name);
	virtual ~AudioPort(){}

	void _cdecl __init__(const string &name);
	virtual void _cdecl __delete__();

	virtual int _cdecl read(AudioBuffer &buf){ return 0; }
	virtual void _cdecl reset(){}
	virtual int _cdecl get_pos(int delta){ return -1; }
};

#endif /* SRC_MODULE_PORT_AUDIOPORT_H_ */
