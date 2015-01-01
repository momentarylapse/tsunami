/*
 * PeakMeter.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PEAKMETER_H_
#define PEAKMETER_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"
#include "../../Stuff/Observable.h"
#include "../../Data/BufferBox.h"

class PeakMeterSource : public Observable
{
public:
	PeakMeterSource(const string &name) : Observable(name){}
	virtual float getSampleRate() = 0;
	virtual void getSomeSamples(BufferBox &buf, int num_samples) = 0;
	virtual int getState() = 0;
	enum{
		STATE_PLAYING,
		STATE_PAUSED,
		STATE_STOPPED
	};
};

class PeakMeter : public HuiEventHandler, public Observer
{
public:
	PeakMeter(HuiPanel *_panel, const string &_id, PeakMeterSource *_source);
	virtual ~PeakMeter();

	void onDraw();
	void onLeftButtonDown();
	void onRightButtonDown();
	void setMode(int _mode);
	void onUpdate(Observable *o, const string &message);
	void enable(bool enabled);

	static const int NUM_SAMPLES;

private:
	void clearData();
	void findPeaks();
	void findSpectrum();

	float i_to_freq(int i);

	HuiPanel *panel;
	string id;
	PeakMeterSource *source;
	int mode;

	enum{
		ModePeaks,
		ModeSpectrum
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

	void drawPeak(HuiPainter *c, const rect &r, Data &d);

	float sample_rate;
	BufferBox buf;

	HuiTimer timer;

	static const int SPECTRUM_SIZE;
	static const float FREQ_MIN;
	static const float FREQ_MAX;
	static const float UPDATE_DT;

	bool enabled;
};

#endif /* PEAKMETER_H_ */
