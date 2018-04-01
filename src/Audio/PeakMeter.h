/*
 * PeakMeter.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_AUDIO_PEAKMETER_H_
#define SRC_AUDIO_PEAKMETER_H_

#include "Source/AudioSource.h"
#include "../Stuff/Observable.h"

class PeakMeterDisplay;
class AudioBuffer;

class PeakMeter : public Observable<AudioSource>
{
	friend class PeakMeterDisplay;
public:
	PeakMeter(AudioSource *s);
	virtual ~PeakMeter();

	AudioSource *source;
	void set_source(AudioSource *s);

	virtual int _cdecl read(AudioBuffer &buf);
	virtual int _cdecl get_pos(int delta);
	virtual int _cdecl sample_rate();
	virtual void _cdecl reset();

	void set_mode(int mode);

	static const int NUM_SAMPLES;

//private:
	void clear_data();
	void find_peaks(AudioBuffer &buf);
	void find_spectrum(AudioBuffer &buf);
	void update(AudioBuffer &buf);

	float i_to_freq(int i);

	int mode;

	enum{
		MODE_PEAKS,
		MODE_SPECTRUM
	};

	struct Data{
		float peak;
		float super_peak, super_peak_t;
		Array<float> spec;

		void reset();
		void update(Array<float> &buf, float dt);
		float get_sp();
	};
	Data r, l;

	int _sample_rate;

	static const int SPECTRUM_SIZE;
	static const float FREQ_MIN;
	static const float FREQ_MAX;
	static const float UPDATE_DT;
};

#endif /* SRC_AUDIO_PEAKMETER_H_ */
