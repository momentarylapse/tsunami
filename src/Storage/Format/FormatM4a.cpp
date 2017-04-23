/*
 * FormatM4a.cpp
 *
 *  Created on: 21.09.2014
 *      Author: michi
 */

#include "FormatM4a.h"
#include "../Storage.h"
#include "../../lib/math/math.h"

FormatDescriptorM4a::FormatDescriptorM4a() :
	FormatDescriptor("Apple lossless audio", "m4a", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ)
{
}

void FormatM4a::loadTrack(StorageOperationData *od)
{
	if (system("which avconv") == 0){
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(("yes | avconv -i \"" + od->filename + "\" \"" + tmp + "\"").c_str());
		od->storage->loadTrack(od->track, tmp, od->offset, od->layer);
		od->storage->current_directory = od->filename.dirname();
		file_delete(tmp);
	}else
		od->error("need external program 'avconv' to decode");
}

