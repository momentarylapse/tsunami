/*
 * date.cpp
 *
 *  Created on: 19 Jul 2022
 *      Author: michi
 */

#include "date.h"

#include <chrono>
#include <ctime>


Date time2date(time_t t) {
	Date d;
	d.time = (int64)t;
	[[maybe_unused]] tm *tm = localtime(&t);
	d.milli_second = 0;
	return d;
}

Date Date::now() {
	auto now = std::chrono::system_clock::now();
	auto t = std::chrono::system_clock::to_time_t(now);
	auto d = time2date(t);

	auto dtn = now.time_since_epoch();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dtn);
	d.milli_second = ms.count() % 1000;
	return d;
}

string Date::format(const string &f) const {
	char buffer [80];
	time_t rawtime = (time_t)this->time;
	tm * timeinfo = localtime (&rawtime);
	strftime(buffer, sizeof(buffer), f.c_str(), timeinfo);
	return buffer;
}

string Date::str() const {
	return this->format("%c");
}

void Date::__assign__(const Date &d) {
	*this = d;
}


