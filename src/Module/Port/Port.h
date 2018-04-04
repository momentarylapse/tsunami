/*
 * Port.h
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_PORT_PORT_H_
#define SRC_MODULE_PORT_PORT_H_

#include "../../lib/base/base.h"

class Port : public VirtualBase
{
public:
	Port();
	virtual ~Port();

	static const int END_OF_STREAM;
	static const int NOT_ENOUGH_DATA;
};

#endif /* SRC_MODULE_PORT_PORT_H_ */
