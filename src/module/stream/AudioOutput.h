/*
 * AudioOutput.h
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#ifndef SRC_DEVICE_STREAM_AUDIOOUTPUT_H_
#define SRC_DEVICE_STREAM_AUDIOOUTPUT_H_



#include "../Module.h"
#include "../ModuleConfiguration.h"
#include "../port/Port.h"
#include "../../lib/base/base.h"
#include "../../lib/base/optional.h"
#include "../../data/audio/RingBuffer.h"
#include "../../device/interface/AudioOutputStream.h"
#include <atomic>

class DeviceManager;
class Device;
class Session;


class AudioOutput : public Module {
public:
	AudioOutput(Session *session);
	virtual ~AudioOutput();

	AudioInPort in{this, "in"};

	void _cdecl __init__(Session *session);
	void __delete__() override;

	void _create_dev();
	void _kill_dev();

	void reset_state() override;


	void stop();
	void start();

	void _pause();
	void _unpause();

	bool is_playing();

	void _fill_prebuffer();

	void set_device(Device *d);
	int get_available();

	float get_volume() const;
	void set_volume(float volume);

	void set_prebuffer_size(int size);

	base::optional<int64> estimate_samples_played();
	int64 get_samples_requested() const;

	base::optional<int> get_latency();

private:
	int _read_stream_into_ring_buffer(int buffer_size);

	AudioOutputStream::SharedData shared_data;

	AudioOutputStream *stream = nullptr;

	DeviceManager *device_manager;


	class Config : public ModuleConfiguration {
	public:
		Device *device;
		float volume;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
	void on_config() override;

	Device *cur_device;
	void update_device();

	enum class State {
		UNPREPARED_NO_DEVICE_NO_DATA,
		UNPREPARED_NO_DEVICE,
		UNPREPARED_NO_DATA,
		PAUSED,
		PLAYING,
	} state;
	void _set_state(State s);

	bool has_data() const;
	bool has_device() const;

	int latency;
	//timeval xxx_prev_time;

	void on_played_end_of_stream();
	void on_read_end_of_stream();


public:
	base::optional<int64> command(ModuleCommand cmd, int64 param) override;
};

#endif /* SRC_DEVICE_STREAM_AUDIOOUTPUT_H_ */
