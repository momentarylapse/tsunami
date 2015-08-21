/*
 * FormatM4a.cpp
 *
 *  Created on: 21.09.2014
 *      Author: michi
 */

#include "FormatM4a.h"
#include "../../Tsunami.h"
#include "../../View/Helper/Progress.h"
#include "../../Stuff/Log.h"
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
	msg_db_f("load_m4a_file", 1);

	if (system("which avconv") == 0){
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(("avconv -i \"" + od->filename + "\" \"" + tmp + "\"").c_str());
		tsunami->storage->loadTrack(od->track, od->filename, od->offset, od->level);
		file_delete(tmp);
	}else
		tsunami->log->error("mp3: need external program 'avconv' to decode");
}

void FormatM4a::saveSong(StorageOperationData *od){}

void FormatM4a::loadSong(StorageOperationData *od)
{
	od->track = od->song->addTrack(Track::TYPE_AUDIO);
	loadTrack(od);
}

