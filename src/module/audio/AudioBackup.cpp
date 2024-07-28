/*
 * AudioBackup.cpp
 *
 *  Created on: 06.03.2019
 *      Author: michi
 */

#include "AudioBackup.h"
#include "../../data/base.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../stuff/BackupManager.h"
#include "../../lib/os/file.h"
#include "../../Session.h"
#include "../../plugins/PluginManager.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

namespace tsunami {

void AudioBackup::Config::reset() {
	channels = 2;
	format = SampleFormat::Float32;
	backup_mode = BackupMode::Temporary;
}

string AudioBackup::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	if (name == "format")
		return "sample-format";
	return "ignore";
}

AudioBackup::AudioBackup(Session *_session) : Module(ModuleCategory::Plumbing, "AudioBackup") {
	session = _session;

	accumulating = true;
	backup_file = nullptr;

	auto _class = session->plugin_manager->get_class("AudioBackupConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("channels", kaba::TypeInt32, &Config::channels);
		kaba::class_add_element("format", kaba::TypeInt32, &Config::format);
		kaba::class_add_element("mode", kaba::TypeInt32, &Config::backup_mode);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;
}

ModuleConfiguration *AudioBackup::get_config() const {
	return (ModuleConfiguration*)&config;
}

int AudioBackup::read_audio(int port, AudioBuffer& buf) {
	auto source = in.source;
	if (!source)
		return Return::NoSource;

	int r = source->read_audio(buf);

	if (r > 0 and accumulating)
		save_chunk(buf.ref(0, r));

	return r;
}

void AudioBackup::set_backup_mode(BackupMode mode) {
	config.backup_mode = mode;
	changed();
}

void AudioBackup::save_chunk(const AudioBuffer &buf) {
	if (backup_file and accumulating) {
		// write to file
		bytes data;
		buf.exports(data, config.channels, config.format);
		backup_file->write(data);
	}
}

void AudioBackup::start() {
	if (backup_file)
		stop();
	if (config.backup_mode != BackupMode::None)
		backup_file = BackupManager::create_file("raw", session);
}

void AudioBackup::stop() {
	if (backup_file) {
		BackupManager::done(backup_file);
		backup_file = nullptr;
		//if (backup_mode != BACKUP_MODE_KEEP)
		//	file_delete(cur_backup_filename);
	}
}

base::optional<int64> AudioBackup::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::Start) {
		start();
		return 0;
	} else if (cmd == ModuleCommand::Stop) {
		stop();
		return 0;
	} else if (cmd == ModuleCommand::AccumulationStart) {
		accumulating = true;
		return 0;
	} else if (cmd == ModuleCommand::AccumulationStop) {
		accumulating = false;
		return 0;
	} else if (cmd == ModuleCommand::SetInputChannels) {
		config.channels = (int)param;
		changed();
		return 0;
	}
	return base::None;
}

}
