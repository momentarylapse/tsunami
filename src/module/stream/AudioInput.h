/*
 * AudioInput.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_DEVICE_STREAM_AUDIOINPUT_H_
#define SRC_DEVICE_STREAM_AUDIOINPUT_H_

#include "../port/Port.h"
#include "../Module.h"
#include "../ModuleConfiguration.h"
#include "../../lib/base/base.h"
#include "../../lib/base/optional.h"
#include "../../data/audio/RingBuffer.h"
#include "../../device/interface/AudioInputStream.h"

namespace tsunami {

class PluginManager;
class DeviceManager;
class Device;
class Session;


class AudioInput : public Module {
	friend class PluginManager;
public:
	AudioInput(Session *session);
	virtual ~AudioInput();

	void _cdecl __init__(Session *session);
	void __delete__() override;

	void _cdecl set_device(Device *device);
	Device* _cdecl get_device();

	void _create_dev();
	void _kill_dev();
	void _pause();
	void _unpause();

	bool _cdecl start();
	void _cdecl stop();


	bool _cdecl is_capturing();

	int _cdecl sample_rate(){ return _sample_rate; }


	AudioOutPort out{this};

	int read_audio(int port, AudioBuffer &buf) override;

	void set_chunk_size(int size);

	int _sample_rate;
	
	base::optional<int64> samples_recorded();

protected:

	DeviceManager *dev_man;

	enum class State {
		NoDevice,
		Capturing,
		Paused,
	} state;


	class Config : public ModuleConfiguration {
	public:
		Device *device;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
	void on_config() override;

	Device *cur_device;
	void update_device();


	AudioInputStream::SharedData shared_data;

	AudioInputStream* stream = nullptr;


public:
	base::optional<int64> command(ModuleCommand cmd, int64 param) override;

};

}

#endif /* SRC_DEVICE_STREAM_AUDIOINPUT_H_ */
