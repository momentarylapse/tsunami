/*
 * PeakMeterDisplay.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PEAKMETERDISPLAY_H_
#define PEAKMETERDISPLAY_H_

#include "../../Stuff/Observable.h"
#include "Graph/Node.h"

class PeakMeter;
class PeakMeterData;
namespace hui {
	class Panel;
}

class PeakMeterDisplay : public scenegraph::Node {
public:

	enum class Mode{
		PEAKS,
		SPECTRUM,
		BOTH
	};
	PeakMeterDisplay(hui::Panel *panel, const string &id, PeakMeter *source, Mode constraint = PeakMeterDisplay::Mode::BOTH);
	PeakMeterDisplay(PeakMeter *source, Mode constraint = PeakMeterDisplay::Mode::BOTH);
	virtual ~PeakMeterDisplay();

	void set_source(PeakMeter *source);

	void on_draw(Painter *p) override;
	bool on_left_button_down() override;
	bool on_right_button_down() override;
	void on_update();
	void enable(bool enabled);

private:

	void connect();
	void unconnect();

	Array<PeakMeterData> channels;

	hui::Panel *panel;
	string id;
	PeakMeter *source;
	int handler_id_draw, handler_id_lbut;
	Mode mode;
	Mode mode_constraint;

	bool enabled;
};

#endif /* PEAKMETERDISPLAY_H_ */
