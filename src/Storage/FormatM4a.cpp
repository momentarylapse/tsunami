/*
 * FormatM4a.cpp
 *
 *  Created on: 21.09.2014
 *      Author: michi
 */

#include "FormatM4a.h"
#include "../Tsunami.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"
#include "Storage.h"
#include "../lib/math/math.h"

FormatM4a::FormatM4a() :
	Format("Apple lossless audio", "m4a", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ)
{
}

FormatM4a::~FormatM4a()
{
}

void FormatM4a::saveBuffer(AudioFile *a, BufferBox *b, const string &filename){}

void FormatM4a::loadTrack(Track *t, const string & filename, int offset, int level)
{
	msg_db_f("load_m4a_file", 1);
	tsunami->progress->set(_("lade m4a"), 0);

	if (system("which avconv") == 0){
		string tmp = "/tmp/tsunami_m4a_out.wav";
		system(("avconv -i \"" + filename + "\" \"" + tmp + "\"").c_str());
		tsunami->storage->loadTrack(t, tmp, offset, level);
		file_delete(tmp);
	}else
		tsunami->log->error("mp3: need external program 'avconv' to decode");
}

void FormatM4a::saveAudio(AudioFile *a, const string & filename){}

void FormatM4a::loadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO);
	loadTrack(t, filename);
}

