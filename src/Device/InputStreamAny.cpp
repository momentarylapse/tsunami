/*
 * InputStreamAny.cpp
 *
 *  Created on: 16.08.2015
 *      Author: michi
 */

#include "../Data/Track.h"
#include "InputStreamAny.h"
#include "InputStreamAudio.h"
#include "InputStreamMidi.h"
#include "DeviceManager.h"


const string InputStreamAny::MESSAGE_CAPTURE = "Capture";
static const int DEFAULT_CHUNK_SIZE = 512;
static const float DEFAULT_UPDATE_TIME = 0.005f;

InputStreamAny::InputStreamAny(const string &name, int _sample_rate) :
	PeakMeterSource(name)
{
	sample_rate = _sample_rate;
	chunk_size = -1;
	update_dt = -1;
	backup_mode = BACKUP_MODE_NONE;
	update_dt = DEFAULT_UPDATE_TIME;
	chunk_size = DEFAULT_CHUNK_SIZE;
}

InputStreamAny::~InputStreamAny()
{
}

void InputStreamAny::setBackupMode(int mode)
{
	backup_mode = mode;
}

void InputStreamAny::setChunkSize(int size)
{
	if (size > 0)
		chunk_size = size;
	else
		chunk_size = DEFAULT_CHUNK_SIZE;
}

void InputStreamAny::setUpdateDt(float dt)
{
	if (dt > 0)
		update_dt = dt;
	else
		update_dt = DEFAULT_UPDATE_TIME;
}
