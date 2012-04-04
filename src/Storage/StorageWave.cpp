/*
 * StorageWave.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "StorageWave.h"

StorageWave::StorageWave() :
	StorageAny("wav", FLAG_SINGLE_TRACK)
{
}

StorageWave::~StorageWave()
{
}

void StorageWave::SaveTrack(Track *t, const string & filename)
{
}



void StorageWave::LoadTrack(Track *t, const string & filename)
{
}

void StorageWave::SaveAudio(AudioFile *a, const string & filename)
{
}



void StorageWave::LoadAudio(AudioFile *a, const string & filename)
{
}




