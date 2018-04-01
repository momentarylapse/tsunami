/*
 * PeakMeterDisplay.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PEAKMETERDISPLAY_H_
#define PEAKMETERDISPLAY_H_

#include "../../Stuff/Observable.h"

class AudioView;
class PeakMeter;
namespace hui{
	class Panel;
}

class PeakMeterDisplay : public VirtualBase
{
public:
	PeakMeterDisplay(hui::Panel *panel, const string &id, PeakMeter *source, AudioView *view);
	virtual ~PeakMeterDisplay();

	void setSource(PeakMeter *source);

	void onDraw(Painter *p);
	void onLeftButtonDown();
	void onRightButtonDown();
	void onUpdate();
	void enable(bool enabled);

private:

	hui::Panel *panel;
	AudioView *view;
	string id;
	PeakMeter *source;

	bool enabled;
};

#endif /* PEAKMETERDISPLAY_H_ */
