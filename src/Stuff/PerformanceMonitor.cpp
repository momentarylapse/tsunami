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

static PerformanceMonitor *perf_mon = NULL;
static std::mutex pm_mutex;
static hui::Timer pm_timer;

#if ALLOW_PERF_MON
struct Channel
{
	bool used;

	string name;
	float t_busy, t_idle;
	float t_last;
	int counter;
	int state;
	void init(const string &_name)
	{
		used = true;
		name = _name;
		reset_state();
	}
	void reset_state()
	{
		t_busy = t_idle = 0;
		t_last = 0;
		state = -1;
		counter = 0;
	}
};
static Array<Channel> channels;

#endif

PerformanceMonitor::PerformanceMonitor()
{
	perf_mon = this;
#if ALLOW_PERF_MON
#endif
}

PerformanceMonitor::~PerformanceMonitor()
{
}

int PerformanceMonitor::create_channel(const string &name)
{
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);
	foreachi(Channel &c, channels, i)
		if (!c.used){
			c.init(name);
			return i;
		}
	Channel c;
	c.init(name);
	channels.add(c);
	return channels.num - 1;
#else
	return -1;
#endif
}

void PerformanceMonitor::delete_channel(int channel)
{
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);
	channels[channel].used = false;
#endif
}

void PerformanceMonitor::start_busy(int channel)
{
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);

	auto &c = channels[channel];
	float t = pm_timer.peek();
	c.t_idle += t - c.t_last;
	c.t_last = t;
	c.state = 1;
#endif
}

void PerformanceMonitor::end_busy(int channel)
{
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

Array<PerformanceMonitor::ChannelInfo> PerformanceMonitor::get_info()
{
	std::lock_guard<std::mutex> lock(pm_mutex);

	float dt = pm_timer.get();

	Array<PerformanceMonitor::ChannelInfo> infos;
	for (auto &c: channels)
		if (c.used){
			ChannelInfo i;
			i.name = c.name;
			i.cpu = c.t_busy / dt;
			i.avg = 0;
			i.counter = c.counter;
			if (c.counter > 0)
				i.avg = c.t_busy / c.counter;
			infos.add(i);
			c.reset_state();
		}
	return infos;
}
