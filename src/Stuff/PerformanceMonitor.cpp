/*
 * PerformanceMonitor.cpp
 *
 *  Created on: 18.09.2017
 *      Author: michi
 */

#include "PerformanceMonitor.h"
#include "../lib/hui/hui.h"

#define ALLOW_PERF_MON	1

static const float UPDATE_DT = 5.0f;
static PerformanceMonitor *perf_mon = NULL;

#if ALLOW_PERF_MON
struct Channel
{
	string name;
	Array<float> cpu_usage;
	float t_busy, t_idle, t_last;
	int n;
	bool used;
	hui::Timer timer;
	int state;
	void reset()
	{
		t_busy = t_idle = t_last = 0;
		state = -1;
		n = 0;
	}
	void update()
	{
		cpu_usage.add(t_busy / (t_busy + t_idle));
	}
};
static Array<Channel> channels;

static void pm_reset()
{
	//timer.get();
	for (auto &c: channels){
		c.t_busy = c.t_idle = c.t_last = 0;
		c.n = 0;
		c.state = -1;
	}
}

static void pm_show()
{
	printf("----- cpu usage -----\n");
	for (auto c: channels)
		if (c.used){
			float avg = 0;
			if ((c.t_busy > 0) and (c.n > 0)){
				c.cpu_usage.add(c.t_busy / (c.t_busy + c.t_idle));
				avg = c.t_busy / c.n;
			}else{
				c.cpu_usage.add(0);
			}
			printf("[%s]: %.1f%%  %.1fms\n", c.name.c_str(), c.cpu_usage.back() * 100.0f, avg * 1000.0f);
		}
	pm_reset();
}
#endif

PerformanceMonitor::PerformanceMonitor()
{
	perf_mon = this;
#if ALLOW_PERF_MON
	pm_reset();
#endif
}

PerformanceMonitor::~PerformanceMonitor()
{
}

int PerformanceMonitor::create_channel(const string &name)
{
#if ALLOW_PERF_MON
	foreachi(Channel &c, channels, i)
		if (!c.used){
			c.used = true;
			c.name = name;
			c.t_busy = c.t_idle = c.t_last = 0;
			c.cpu_usage.clear();
			c.state = -1;
			return i;
		}
	Channel c;
	c.n = 0;
	c.name = name;
	c.t_busy = c.t_idle = c.t_last = 0;
	c.cpu_usage.clear();
	c.state = -1;
	c.used = true;
	channels.add(c);
	return channels.num - 1;
#else
	return -1;
#endif
}

void PerformanceMonitor::delete_channel(int channel)
{
#if ALLOW_PERF_MON
	channels[channel].used = false;
#endif
}

void PerformanceMonitor::start_busy(int channel)
{
#if ALLOW_PERF_MON
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
	//float t = timer.peek();
	auto &c = channels[channel];
	//printf("- %d %f\n", channel, t);
	float dt = c.timer.get();
	c.t_busy += dt;// t - c.t_last;
	c.n ++;
	//c.t_last = t;
	c.state = 0;
	if (c.t_busy + c.t_idle > UPDATE_DT){
		pm_show();
		//hui::RunLater(0.01f, std::bind(&PerformanceMonitor::notify, perf_mon, &PerformanceMonitor::MESSAGE_CHANGE));
		hui::RunLater(0.01f, &pm_notify);
	}
#endif
}

Array<PerformanceMonitor::ChannelInfo> PerformanceMonitor::get_info()
{
	Array<PerformanceMonitor::ChannelInfo> infos;
	for (auto &c: channels)
		if (c.used){
			ChannelInfo i;
			i.name = c.name;
			i.cpu = c.cpu_usage;
		}
	return infos;
}
