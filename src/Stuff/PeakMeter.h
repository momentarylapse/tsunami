/*
 * PeakMeter.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PEAKMETER_H_
#define PEAKMETER_H_

#include "../lib/hui/hui.h"

class PeakMeter : public HuiEventHandler
{
public:
	PeakMeter(CHuiWindow *_win, const string &_id);
	virtual ~PeakMeter();

	void Set(float _peak_r, float _peak_l);

	void OnDraw();

private:
	CHuiWindow *win;
	string id;

	float peak_r, peak_l;
};

#endif /* PEAKMETER_H_ */
