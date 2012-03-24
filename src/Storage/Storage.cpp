/*
 * Storage.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Storage.h"
#include "StorageWave.h"

Storage::Storage()
{
	format.add(new StorageWave());
}

Storage::~Storage()
{
	// TODO Auto-generated destructor stub
}
