/*
 * PerformanceMonitor.cpp
 *
 *  Created on: 18.09.2017
 *      Author: michi
 */

#include "PerformanceMonitor.h"
#include "../lib/hui/hui.h"
#include <mutex>

#define ALLOW_PERF_MON	1

static const float UPDATE_DT = 2.0f;


static PerformanceMonitor *pm_instance;
static Array<PerfChannelInfo> pm_info;
static std::mutex pm_mutex;
static hui::Timer pm_timer;

#if ALLOW_PERF_MON
struct Channel {
	bool used;

	string name;
	void *p;
	
	float t_busy, t_idle;
	float t_last;
	int counter;
	int state;


	Array<PerfChannelStat> history;

	void init(const string &_name, void *_p) {
		used = true;
		name = _name;
		p = _p;
		history.clear();
		reset_state();
	}
	void reset_state() {
		t_busy = t_idle = 0;
		t_last = 0;
		state = -1;
		counter = 0;
	}
};
static Array<Channel> channels;

#endif

// single instance!
PerformanceMonitor::PerformanceMonitor() {
	pm_instance = this;
#if ALLOW_PERF_MON
	runner_id = hui::RunRepeated(UPDATE_DT, std::bind(&PerformanceMonitor::update, this));
#else
	runner_id = -1;
#endif
}

PerformanceMonitor::~PerformanceMonitor() {
#if ALLOW_PERF_MON
	hui::CancelRunner(runner_id);
#endif
}

int PerformanceMonitor::create_channel(const string &name, void *p) {
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);
	foreachi(Channel &c, channels, i)
		if (!c.used) {
			c.init(name, p);
			return i;
		}
	Channel c;
	c.init(name, p);
	channels.add(c);
	return channels.num - 1;
#else
	return -1;
#endif
}

void PerformanceMonitor::delete_channel(int channel) {
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);
	channels[channel].used = false;
	channels[channel].history.clear();
#endif
}

void PerformanceMonitor::start_busy(int channel) {
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);

	auto &c = channels[channel];
	float t = pm_timer.peek();
	c.t_idle += t - c.t_last;
	c.t_last = t;
	c.state = 1;
#endif
}

void PerformanceMonitor::end_busy(int channel) {
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);

	auto &c = channels[channel];
	float t = pm_timer.peek();
	c.t_busy += t - c.t_last;
	c.t_last = t;
	c.counter ++;
	c.state = 0;
#endif
}

// called in main thread
void PerformanceMonitor::update() {
#if ALLOW_PERF_MON
	{
	std::lock_guard<std::mutex> lock(pm_mutex);

	float dt = pm_timer.get();

	pm_info.clear();
	for (auto &c: channels)
		if (c.used) {

			PerfChannelStat s;
			s.cpu = c.t_busy / dt;
			s.avg = 0;
			s.counter = c.counter;
			if (c.counter > 0)
				s.avg = c.t_busy / c.counter;
			c.history.add(s);

			PerfChannelInfo i;
			i.name = c.name;
			i.p = c.p;
			i.stats = c.history;
			pm_info.add(i);
			c.reset_state();
		}
	}

	notify();
#endif
}

// call from main thread!!!
Array<PerfChannelInfo> PerformanceMonitor::get_info() {
	//std::lock_guard<std::mutex> lock(pm_mutex);
	return pm_info;
}
