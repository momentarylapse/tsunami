/*
 * PeakMeter.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_PEAKMETER_H_
#define SRC_MODULE_AUDIO_PEAKMETER_H_

#include "AudioVisualizer.h"

namespace tsunami {

class PeakMeterDisplay;
class AudioBuffer;


struct PeakMeterData {
	float peak;
	float super_peak, super_peak_t;
	Array<float> spec;

	PeakMeterData();
	void reset();
	void update(Array<float> &buf, float dt);
	float get_sp();
};

class PeakMeter : public AudioVisualizer {
	friend class PeakMeterDisplay;
public:
	PeakMeter();
	~PeakMeter() override;

	void process(AudioBuffer &buf) override;
	void reset_state() override;

	int spectrum_requests;
	void request_spectrum();
	void unrequest_spectrum();

//private:
	void clear_data();
	void _set_channels(int num_channels);
	void find_peaks(AudioBuffer &buf);
	void find_spectrum(AudioBuffer &buf);

	float i_to_freq(int i);

	Array<PeakMeterData> channels[2];
	Array<PeakMeterData> read_channels();


	ConfigPanel *create_panel() override;

	static const int SPECTRUM_SIZE;
	static const float FREQ_MIN;
	static const float FREQ_MAX;
};

}

#endif /* SRC_MODULE_AUDIO_PEAKMETER_H_ */
