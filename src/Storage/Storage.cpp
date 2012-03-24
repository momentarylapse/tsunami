/*
 * Storage.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Storage.h"
#include "StorageWave.h"
#include "StorageNami.h"
#include "../Tsunami.h"

Storage::Storage()
{
	format.add(new StorageNami());
	format.add(new StorageWave());
}

Storage::~Storage()
{
	// TODO Auto-generated destructor stub
}

void Storage::Load(AudioFile *a, const string &filename)
{
	tsunami->CurrentDirectory = dirname(filename);
}
