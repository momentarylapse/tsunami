/*
 * UnitTest.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "UnitTest.h"
#include "../lib/file/msg.h"
#include "../lib/hui/hui.h"
#include "../Data/Audio/AudioBuffer.h"
#include <math.h>

#include "TestAudioBuffer.h"
#include "TestRingBuffer.h"
#include "TestInterpolator.h"
#include "TestRhythm.h"
#include "TestStreams.h"
#include "TestThreads.h"
#include "TestMidiPreview.h"
#include "TestPlugins.h"
#include "TestMixer.h"

UnitTest::UnitTest(const string &_name)
{
	name = _name;
}

UnitTest::~UnitTest()
{
}

void UnitTest::run(const string &filter, TestProtocoll &protocoll)
{
	msg_write("<<<<<<<<<<<<<<<<<<<<  " + name + "  >>>>>>>>>>>>>>>>>>>>");
	for (auto &t: tests()){
		if (!filter_match(filter, t.name))
			continue;
		msg_write("== " + t.name + " ==");
		try{
			t.f();
			msg_write("  ok");
		}catch(Failure &e){
			msg_error(e.message());
			protocoll.num_failed ++;
		}
		protocoll.num_tests_run ++;
	}
}

bool UnitTest::filter_match(const string &filter, const string &test_name)
{
	if (!filter_match_group(filter))
		return false;
	auto xx = filter.explode("/");
	if (xx.num < 2)
		return true;
	return test_name.match(xx[1]);
}

bool UnitTest::filter_match_group(const string &filter)
{
	auto xx = filter.explode("/");
	if (xx[0] == "all" or xx[0] == "*")
		return true;
	if (name.match(xx[0]))
		return true;
	return false;
}

void UnitTest::sleep(float t)
{
	hui::Timer timer;
	while (timer.peek() < t){
		hui::Application::do_single_main_loop();
		hui::Sleep(0.001f);
	}
}

AudioBuffer UnitTest::make_buf(const Array<float> &r, const Array<float> &l)
{
	AudioBuffer buf;
	buf.resize(r.num);
	buf.c[0] = r;
	buf.c[1] = l;
	if (l.num != r.num)
		throw Failure("INTERNAL: make_buf() r.len != l.len");
	return buf;
}

void UnitTest::assert_equal(float a, float b, float epsilon)
{
	if (fabs(a - b) > epsilon)
		throw Failure("a!=b\na: " + f2s(a, 6) + "\nb: " + f2s(b, 6));
}

void UnitTest::assert_equal(const Array<int> &a, const Array<int> &b)
{
	if (a.num != b.num)
		throw Failure(format("a.num (%d) != b.num (%d)", a.num, b.num));
	for (int i=0; i<a.num; i++)
		if (a[i] != b[i])
			throw Failure("a!=b\na: " + ia2s(a) + "\nb: " + ia2s(b));
}

void UnitTest::assert_equal(const Array<float> &a, const Array<float> &b, float epsilon)
{
	if (a.num != b.num)
		throw Failure(format("a.num (%d) != b.num (%d)", a.num, b.num));
	for (int i=0; i<a.num; i++)
		if (fabs(a[i] - b[i]) > epsilon)
			throw Failure("a!=b\na: " + fa2s(a) + "\nb: " + fa2s(b));
}

void UnitTest::assert_equal(const AudioBuffer &a, const AudioBuffer &b, float epsilon)
{
	if (a.length != b.length)
		throw Failure(format("a.length (%d) != b.length (%d)", a.length, b.length));
	if (a.channels != b.channels)
		throw Failure(format("a.channels (%d) != b.channels(%d)", a.channels, b.channels));
	for (int ci=0; ci<a.channels; ci++)
		for (int i=0; i<a.length; i++)
			if (fabs(a.c[ci][i] - b.c[ci][i]) > epsilon)
				throw Failure("a!=b (channel " + i2s(ci) + "\na: " + fa2s(a.c[ci]) + "\nb: " + fa2s(b.c[ci]));

}

Array<UnitTest*> UnitTest::all()
{
	Array<UnitTest*> tests;
	tests.add(new TestAudioBuffer);
	tests.add(new TestRingBuffer);
	tests.add(new TestInterpolator);
	tests.add(new TestRhythm);
	tests.add(new TestMixer);
	tests.add(new TestThreads);
	tests.add(new TestStreams);
	tests.add(new TestMidiPreview);
	tests.add(new TestPlugins);
	return tests;
}

void UnitTest::run_all(const string &filter)
{
	auto tests = all();

	TestProtocoll protocoll;

	for (auto *t: tests)
		if (t->filter_match_group(filter))
			t->run(filter, protocoll);

	for (auto *t: tests)
		delete t;


	msg_write("\n\n");
	if (protocoll.num_failed > 0){
		msg_error(format("%d out of %d tests failed", protocoll.num_failed, protocoll.num_tests_run));
	}else{
		msg_write("-----------------------------");
		msg_write(format("all %d tests succeeded", protocoll.num_tests_run));
		msg_write("-----------------------------");
	}
}

void UnitTest::print_all_names()
{
	auto tests = all();

	msg_write("available tests:");
	for (auto *t: tests)
		msg_write("  " + t->name);
	msg_write("  all");

	for (auto *t: tests)
		delete t;
}
#endif
