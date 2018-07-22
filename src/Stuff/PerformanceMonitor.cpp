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

static const float UPDATE_DT = 1.0f;
static PerformanceMonitor *perf_mon = NULL;
static std::mutex pm_mutex;

#if ALLOW_PERF_MON
struct Channel
{
	string name;
	float t_busy, t_idle, t_last;
	int n;
	bool used;
	hui::Timer timer;
	int state;
	float _cpu, _avg;
	void init(const string &_name)
	{
		name = _name;
		used = true;
		reset_state();
		_cpu = 0;
		_avg = 0;
	}
	void reset_state()
	{
		t_busy = t_idle = t_last = 0;
		state = -1;
		n = 0;
	}
	void update()
	{
		_cpu = t_busy / (t_busy + t_idle);
		_avg = 0;
		if ((t_busy > 0) and (n > 0))
			_avg = t_busy / n;
		reset_state();
	}
};
static Array<Channel> channels;

static void pm_update()
{
	for (auto &c: channels)
		if (c.used)
			c.update();
}

static void pm_show()
{
	printf("----- cpu usage -----\n");
	for (auto &c: channels)
		if (c.used)
			printf("[%s]: %.1f%%  %.1fms\n", c.name.c_str(), c._cpu * 100.0f, c._avg * 1000.0f);
}
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
	//float t = timer.peek();
	//printf("+ %d %f\n", channel, t);
	auto &c = channels[channel];
	float dt = c.timer.get();
	c.t_idle += dt;//t - c.t_last;
	//c.t_last = t;
	c.state = 1;
	//if (t > 5)
	//	show();
#endif
}

void pm_notify()
{
	perf_mon->notify();
}

void PerformanceMonitor::end_busy(int channel)
{
#if ALLOW_PERF_MON
	std::lock_guard<std::mutex> lock(pm_mutex);

	//float t = timer.peek();
	auto &c = channels[channel];
	//printf("- %d %f\n", channel, t);
	float dt = c.timer.get();
	c.t_busy += dt;// t - c.t_last;
	c.n ++;
	//c.t_last = t;
	c.state = 0;
	if (c.t_busy + c.t_idle > UPDATE_DT){
		pm_update();
		//pm_show();
		//hui::RunLater(0.01f, std::bind(&PerformanceMonitor::notify, perf_mon, &PerformanceMonitor::MESSAGE_CHANGE));
		hui::RunLater(0.01f, &pm_notify);
	}
#endif
}

Array<PerformanceMonitor::ChannelInfo> PerformanceMonitor::get_info()
{
	std::lock_guard<std::mutex> lock(pm_mutex);
	Array<PerformanceMonitor::ChannelInfo> infos;
	for (auto &c: channels)
		if (c.used){
			ChannelInfo i;
			i.name = c.name;
			i.cpu = c._cpu;
			i.avg = c._avg;
			infos.add(i);
		}
	return infos;
}
