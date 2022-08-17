/*
 * PerformanceMonitor.cpp
 *
 *  Created on: 18.09.2017
 *      Author: michi
 */

#include "PerformanceMonitor.h"
#include "../lib/hui/hui.h"
#include "../lib/os/time.h"
#include "../module/Module.h"
#include <mutex>

#define ALLOW_PERF_MON	1

static const float UPDATE_DT = 2.0f;


namespace os {
	extern void require_main_thread(const string &msg);
}

// on a slow computer, a group of
//   start_busy()
//   end_busy()
// added ~35ns of overhead


static PerformanceMonitor *pm_instance;
static Array<PerfChannelInfo> pm_info;
static std::mutex pm_mutex;
static os::Timer pm_timer;

#if ALLOW_PERF_MON
struct Channel {
	bool used;

	string name;
	void *p;
	int parent;
	int id;
	
	float t_busy, t_idle;
	float t_last;
	int counter;
	int state;


	Array<PerfChannelStat> history;

	void init(const string &_name, void *_p, int _id) {
		used = true;
		name = _name;
		p = _p;
		id = _id;
		parent = -1;
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
	runner_id = hui::run_repeated(UPDATE_DT, [this]{ update(); });
#else
	runner_id = -1;
#endif
}

PerformanceMonitor::~PerformanceMonitor() {
#if ALLOW_PERF_MON
	hui::cancel_runner(runner_id);
#endif
}

int PerformanceMonitor::create_channel(const string &name, void *p) {
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);
	foreachi(Channel &c, channels, i)
		if (!c.used) {
			c.init(name, p, i);
			return i;
		}
	Channel c;
	c.init(name, p, channels.num);
	channels.add(c);
	return channels.num - 1;
#else
	return -1;
#endif
}

void PerformanceMonitor::set_parent(int channel, int parent) {
	if (channel < 0)
		return;
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);
	channels[channel].parent = parent;
#endif
}

void PerformanceMonitor::set_name(int channel, const string &name) {
	if (channel < 0)
		return;
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);
	channels[channel].name = name;
#endif
}

string PerformanceMonitor::get_name(int channel) {
	if (channel < 0 or channel >= channels.num)
		return "";
	return channels[channel].name;
}

void PerformanceMonitor::delete_channel(int channel) {
	if (channel < 0)
		return;
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
			i.id = c.id;
			i.parent = c.parent;
			i.stats = c.history;
			pm_info.add(i);
			c.reset_state();
		}
	}

	notify();
#endif
}

Array<PerfChannelInfo> PerformanceMonitor::get_info() {
	os::require_main_thread("PerfMon.info");
	
	//std::lock_guard<std::mutex> lock(pm_mutex);
	return pm_info;
}
