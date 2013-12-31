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
	virtual float GetSampleRate() = 0;
	virtual void GetSomeSamples(BufferBox &buf, int num_samples) = 0;
};

class PeakMeter : public HuiEventHandler, public Observer
{
public:
	PeakMeter(HuiWindow *_win, const string &_id, PeakMeterSource *_source);
	virtual ~PeakMeter();

	void OnDraw();
	void OnLeftButtonDown();
	void OnRightButtonDown();
	void SetMode(int _mode);
	void OnUpdate(Observable *o);

private:
	void FindPeaks();
	void FindSpectrum();

	HuiWindow *win;
	string id;
	PeakMeterSource *source;
	int mode;

	enum{
		ModePeaks,
		ModeSpectrum
	};

	float sample_rate;
	BufferBox buf;
	float peak_r, peak_l;
	Array<float> spec_r, spec_l;
	float super_peak_r, super_peak_l;
	float super_peak_r_t, super_peak_l_t;
};

#endif /* PEAKMETER_H_ */
