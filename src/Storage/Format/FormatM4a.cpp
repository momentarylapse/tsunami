/*
 * FormatM4a.cpp
 *
 *  Created on: 21.09.2014
 *      Author: michi
 */

#include "FormatM4a.h"
#include "../Storage.h"
#include "../../lib/math/math.h"

FormatM4a::FormatM4a() :
	Format("Apple lossless audio", "m4a", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ)
{
}

FormatM4a::~FormatM4a()
{
}

void FormatM4a::saveBuffer(StorageOperationData *od){}

void FormatM4a::loadTrack(StorageOperationData *od)
{
	if (system("which avconv") == 0){
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(("avconv -i \"" + od->filename + "\" \"" + tmp + "\"").c_str());
		od->storage->loadTrack(od->track, tmp, od->offset, od->level);
		od->storage->current_directory = od->filename.dirname();
		file_delete(tmp);
	}else
		od->error("mp3: need external program 'avconv' to decode");
}

void FormatM4a::saveSong(StorageOperationData *od){}

void FormatM4a::loadSong(StorageOperationData *od)
{
	od->track = od->song->addTrack(Track::TYPE_AUDIO);
	loadTrack(od);
}

