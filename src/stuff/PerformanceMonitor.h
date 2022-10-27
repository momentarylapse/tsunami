/*
 * PerformanceMonitor.h
 *
 *  Created on: 18.09.2017
 *      Author: michi
 */

#ifndef SRC_STUFF_PERFORMANCEMONITOR_H_
#define SRC_STUFF_PERFORMANCEMONITOR_H_

#include "../lib/base/base.h"
#include "../lib/pattern/Observable.h"


struct PerfChannelStat {
	float cpu, avg;
	int counter;
};

class PerfChannelInfo {
public:
	int id, parent;
	string name;
	void *p;
	Array<PerfChannelStat> stats;
};

class PerformanceMonitor : public Observable<VirtualBase> {
public:
	PerformanceMonitor();
	virtual ~PerformanceMonitor();

	void update();
	int runner_id;
	static Array<PerfChannelInfo> get_info();

	static int create_channel(const string &name, void *p);
	static void set_name(int channel, const string &name);
	static string get_name(int channel);
	static void set_parent(int channel, int parent);
	static void delete_channel(int channel);
	static void start_busy(int channel);
	static void end_busy(int channel);
};

#endif /* SRC_STUFF_PERFORMANCEMONITOR_H_ */
