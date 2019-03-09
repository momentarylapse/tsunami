/*
 * AudioBackup.h
 *
 *  Created on: 06.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOBACKUP_H_
#define SRC_MODULE_AUDIO_AUDIOBACKUP_H_

#include "../Port/Port.h"
#include "../Module.h"

class AudioBackup : public Module
{
public:
	AudioBackup(Session *session);
	~AudioBackup();

	class Output : public Port
	{
	public:
		Output(AudioBackup *j);
		int read_audio(AudioBuffer &buf) override;
		AudioBackup *backup;
	};
	Output *out;

	Port *source;

	void _cdecl set_backup_mode(int mode);
	int backup_mode;
	File *backup_file;

	void save_chunk(AudioBuffer &buf);

	void start();
	void stop();
	int command(ModuleCommand cmd, int param) override;
};

#endif /* SRC_MODULE_AUDIO_AUDIOBACKUP_H_ */
