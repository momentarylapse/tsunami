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
	virtual int GetState() = 0;
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

	void OnDraw();
	void OnLeftButtonDown();
	void OnRightButtonDown();
	void SetMode(int _mode);
	void OnUpdate(Observable *o, const string &message);
	void Enable(bool enabled);

private:
	void ClearData();
	void FindPeaks();
	void FindSpectrum();

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
		void update(Array<float> &buf, float sample_rate);
		float get_sp();
	};
	Data r, l;

	void DrawPeak(HuiPainter *c, const rect &r, Data &d);

	float sample_rate;
	BufferBox buf;

	bool enabled;
};

#endif /* PEAKMETER_H_ */
