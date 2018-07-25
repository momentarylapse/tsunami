/*
 * Port.h
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_PORT_PORT_H_
#define SRC_MODULE_PORT_PORT_H_

#include "../../lib/base/base.h"

enum class SignalType;

class Port : public VirtualBase
{
public:
	Port();
	virtual ~Port();

	static const int END_OF_STREAM;
	static const int NOT_ENOUGH_DATA;
};

class PortDescription
{
public:
	PortDescription(){}
	PortDescription(SignalType type, Port **port, const string &name);
	SignalType type;
	Port **port;
	string name;
};

#endif /* SRC_MODULE_PORT_PORT_H_ */
