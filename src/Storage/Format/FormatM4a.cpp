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
	FormatDescriptor("Apple lossless audio", "m4a", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::TAGS | Flag::READ)
{
}

void FormatM4a::load_track(StorageOperationData *od)
{
	if (system("which avconv") == 0){
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(("yes | avconv -i \"" + od->filename + "\" \"" + tmp + "\"").c_str());
		od->storage->load_track(od->layer, tmp, od->offset);
		od->storage->current_directory = path_dirname(od->filename);
		file_delete(tmp);
	}else if (system("which ffmpeg") == 0){
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(("yes | ffmpeg -i \"" + od->filename + "\" \"" + tmp + "\"").c_str());
		od->storage->load_track(od->layer, tmp, od->offset);
		od->storage->current_directory = path_dirname(od->filename);
		file_delete(tmp);
	}else
		od->error("need external program 'avconv' or 'ffmpeg' to decode");
}

