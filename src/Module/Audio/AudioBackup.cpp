/*
 * AudioBackup.cpp
 *
 *  Created on: 06.03.2019
 *      Author: michi
 */

#include "AudioBackup.h"
#include "../../Data/base.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Stuff/BackupManager.h"

AudioBackup::AudioBackup(Session *_session) : Module(ModuleType::PLUMBING, "AudioBackup")
{
	session = _session;
	out = new Output(this);
	port_out.add(out);
	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));
	source = nullptr;

	backup_file = nullptr;
	backup_mode = BACKUP_MODE_TEMP;
}

AudioBackup::~AudioBackup()
{
}

int AudioBackup::Output::read_audio(AudioBuffer& buf)
{
	if (!backup->source)
		return buf.length;

	int r = backup->source->read_audio(buf);

	if (r > 0)
		backup->save_chunk(buf.ref(0, r));

	return r;
}

AudioBackup::Output::Output(AudioBackup *b) : Port(SignalType::AUDIO, "out")
{
	backup = b;
}

void AudioBackup::set_backup_mode(int mode)
{
	backup_mode = mode;
}

void AudioBackup::save_chunk(const AudioBuffer &buf)
{
	if (backup_file){
		// write to file
		string data;
		buf.exports(data, 2, SampleFormat::SAMPLE_FORMAT_32_FLOAT);
		backup_file->write_buffer(data);
	}
}

void AudioBackup::start()
{
	if (backup_file)
		stop();
	if (backup_mode != BACKUP_MODE_NONE)
		backup_file = BackupManager::create_file("raw", session);
}

void AudioBackup::stop()
{
	if (backup_file){
		BackupManager::done(backup_file);
		backup_file = nullptr;
		//if (backup_mode != BACKUP_MODE_KEEP)
		//	file_delete(cur_backup_filename);
	}
}

int AudioBackup::command(ModuleCommand cmd, int param)
{
	if (cmd == ModuleCommand::START){
		start();
		return 0;
	}else if (cmd == ModuleCommand::STOP){
		stop();
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}
