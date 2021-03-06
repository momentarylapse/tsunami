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
#include "../ModuleConfiguration.h"

class File;

enum class BackupMode;
enum class SampleFormat;

class AudioBackup : public Module {
public:
	AudioBackup(Session *session);

	class Output : public Port {
	public:
		Output(AudioBackup *j);
		int read_audio(AudioBuffer &buf) override;
		AudioBackup *backup;
	};

	Port *source;

	void _cdecl set_backup_mode(BackupMode mode);
	File *backup_file;
	bool accumulating;

	void save_chunk(const AudioBuffer &buf);

	void start();
	void stop();
	int command(ModuleCommand cmd, int param) override;



	class Config : public ModuleConfiguration {
	public:
		BackupMode backup_mode;
		int channels;
		SampleFormat format;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
};

#endif /* SRC_MODULE_AUDIO_AUDIOBACKUP_H_ */
