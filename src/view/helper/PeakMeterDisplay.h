/*
 * PeakMeterDisplay.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PEAKMETERDISPLAY_H_
#define PEAKMETERDISPLAY_H_

#include "../../lib/pattern/Observable.h"
#include "graph/Node.h"

class PeakMeter;
struct PeakMeterData;
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
	bool on_left_button_down(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;
	void on_update();
	void enable(bool enabled);
	void set_channel_map(const Array<int> &channel_map);

	static int good_size(int num_channels);

	void set_visible(bool visible);

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

	Array<int> channel_map;

	static const int SPACE_BETWEEN_CHANNELS;
	static const int CHANNEL_SIZE_RECOMMENDED;
};

#endif /* PEAKMETERDISPLAY_H_ */
