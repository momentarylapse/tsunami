/*
 * FormatM4a.cpp
 *
 *  Created on: 21.09.2014
 *      Author: michi
 */

#include "FormatM4a.h"
#include "../Storage.h"
#include "../../lib/math/math.h"
#include "../../lib/os/filesystem.h"
#include "../../lib/os/msg.h"

FormatDescriptorM4a::FormatDescriptorM4a() :
	FormatDescriptor("Apple lossless audio", "m4a", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::TAGS | Flag::READ)
{
}

void FormatM4a::load_track(StorageOperationData *od) {
	if (system("which avconv") == 0) {
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(format("yes | avconv -i \"%s\" \"%s\"", od->filename, tmp).c_str());
		od->storage->load_track(od->layer, tmp, od->offset);
		od->storage->current_directory = od->filename.parent();
		os::fs::_delete(tmp);
	} else if (system("which ffmpeg") == 0) {
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(format("yes | ffmpeg -i \"%s\" \"%s\"", od->filename, tmp).c_str());
		od->storage->load_track(od->layer, tmp, od->offset);
		od->storage->current_directory = od->filename.parent();
		os::fs::_delete(tmp);
	} else {
		od->error("need external program 'avconv' or 'ffmpeg' to decode");
	}
}

