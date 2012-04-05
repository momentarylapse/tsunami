/*
 * StorageFlac.cpp
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#include "StorageFlac.h"

StorageFlac::StorageFlac() :
	StorageAny("flac", FLAG_SINGLE_TRACK | FLAG_TAGS)
{
}

StorageFlac::~StorageFlac()
{
}

void StorageFlac::LoadTrack(Track *t, const string & filename)
{
}



void StorageFlac::SaveAudio(AudioFile *a, const string & filename)
{
}



void StorageFlac::SaveBuffer(AudioFile *a, BufferBox *b, const string & filename)
{
}



void StorageFlac::LoadAudio(AudioFile *a, const string & filename)
{
}


