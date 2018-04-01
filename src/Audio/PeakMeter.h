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
	void setSource(AudioSource *s);

	virtual int _cdecl read(AudioBuffer &buf);
	virtual int _cdecl getPos(int delta);
	virtual int _cdecl getSampleRate();

	void setMode(int mode);

	static const int NUM_SAMPLES;

//private:
	void clearData();
	void findPeaks(AudioBuffer &buf);
	void findSpectrum(AudioBuffer &buf);
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

	float sample_rate;

	static const int SPECTRUM_SIZE;
	static const float FREQ_MIN;
	static const float FREQ_MAX;
	static const float UPDATE_DT;
};

#endif /* SRC_AUDIO_PEAKMETER_H_ */
