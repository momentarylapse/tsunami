/*
 * Port.cpp
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#include "Port.h"

const int Port::NOT_ENOUGH_DATA = 0;
const int Port::END_OF_STREAM = -2;

Port::Port(SignalType _type, const string &_name)
{
	type = _type;
	name = _name;
}

InPortDescription::InPortDescription(SignalType _type, Port** _port, const string& _name)
{
	type = _type;
	port = _port;
	name = _name;
}
