/*
 * CpuDisplay.h
 *
 *  Created on: 10.03.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_CPUDISPLAY_H_
#define SRC_VIEW_HELPER_CPUDISPLAY_H_

#include "../../Stuff/Observable.h"

namespace hui{
	class Panel;
}
class Painter;
class PerformanceMonitor;
class Session;
class AudioView;

class CpuDisplay : public VirtualBase
{
public:
	CpuDisplay(hui::Panel *panel, const string &id, Session *session);
	virtual ~CpuDisplay();

	void onDraw(Painter *p);
	void onUpdate();

	PerformanceMonitor *perf_mon;
	AudioView *view;

	hui::Panel *panel;
	string id;

	enum{
		TYPE_VIEW,
		TYPE_OUT,
		TYPE_SUCK,
		NUM_TYPES
	};

	Array<float> cpu[NUM_TYPES];
	Array<float> avg[NUM_TYPES];
};

#endif /* SRC_VIEW_HELPER_CPUDISPLAY_H_ */
