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
#include "../../lib/file/file.h"
#include "../../Session.h"
#include "../../plugins/PluginManager.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}



void AudioBackup::Config::reset() {
	channels = 2;
	format = SampleFormat::SAMPLE_FORMAT_32_FLOAT;
	backup_mode = BackupMode::TEMP;
}

string AudioBackup::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}

AudioBackup::AudioBackup(Session *_session) : Module(ModuleCategory::PLUMBING, "AudioBackup") {
	session = _session;
	port_out.add(new Output(this));
	port_in.add({SignalType::AUDIO, &source, "in"});
	source = nullptr;

	accumulating = true;
	backup_file = nullptr;

	auto _class = session->plugin_manager->get_class("AudioBackupConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("channels", kaba::TypeInt, &Config::channels);
		kaba::class_add_element("format", kaba::TypeInt, &Config::format);
		kaba::class_add_element("mode", kaba::TypeInt, &Config::backup_mode);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;
}

ModuleConfiguration *AudioBackup::get_config() const {
	return (ModuleConfiguration*)&config;
}

int AudioBackup::Output::read_audio(AudioBuffer& buf) {
	if (!backup->source)
		return NO_SOURCE;

	int r = backup->source->read_audio(buf);

	if (r > 0 and backup->accumulating)
		backup->save_chunk(buf.ref(0, r));

	return r;
}

AudioBackup::Output::Output(AudioBackup *b) : Port(SignalType::AUDIO, "out") {
	backup = b;
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
	if (config.backup_mode != BackupMode::NONE)
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

int AudioBackup::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::START) {
		start();
		return 0;
	} else if (cmd == ModuleCommand::STOP) {
		stop();
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_START) {
		accumulating = true;
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_STOP) {
		accumulating = false;
		return 0;
	} else if (cmd == ModuleCommand::SET_INPUT_CHANNELS) {
		config.channels = param;
		changed();
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}
