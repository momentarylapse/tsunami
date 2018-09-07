/*
 * PeakMeterDisplay.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PEAKMETERDISPLAY_H_
#define PEAKMETERDISPLAY_H_

#include "../../Stuff/Observable.h"

class PeakMeter;
class PeakMeterData;
namespace hui{
	class Panel;
}

class PeakMeterDisplay : public VirtualBase
{
public:
	PeakMeterDisplay(hui::Panel *panel, const string &id, PeakMeter *source);
	virtual ~PeakMeterDisplay();

	void set_source(PeakMeter *source);

	void on_draw(Painter *p);
	void on_left_button_down();
	void on_right_button_down();
	void on_update();
	void enable(bool enabled);

private:

	PeakMeterData *r, *l;

	hui::Panel *panel;
	string id;
	PeakMeter *source;
	int handler_id_draw, handler_id_lbut;

	bool enabled;
};

#endif /* PEAKMETERDISPLAY_H_ */
