/*
 * PerformanceMonitor.h
 *
 *  Created on: 18.09.2017
 *      Author: michi
 */

#ifndef SRC_STUFF_PERFORMANCEMONITOR_H_
#define SRC_STUFF_PERFORMANCEMONITOR_H_

#include "../lib/base/base.h"
#include "Observable.h"

class PerformanceMonitor : public Observable<VirtualBase>
{
public:
	PerformanceMonitor();
	virtual ~PerformanceMonitor();

	struct ChannelInfo{
		string name;
		float cpu, avg;
		int counter;
	};
	void update();
	int runner_id;
	static Array<ChannelInfo> get_info();

	static int create_channel(const string &name);
	static void delete_channel(int channel);
	static void start_busy(int channel);
	static void end_busy(int channel);
};

#endif /* SRC_STUFF_PERFORMANCEMONITOR_H_ */
