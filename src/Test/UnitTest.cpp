/*
 * UnitTest.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#include "UnitTest.h"
#include "../lib/file/msg.h"
#include "../lib/hui/hui.h"
#include <unistd.h>

UnitTest::UnitTest(const string &_name)
{
	name = _name;
}

UnitTest::~UnitTest()
{
}

void UnitTest::run()
{
	msg_write("<<<<<<<<<<<<<<<<<<<<  " + name + "  >>>>>>>>>>>>>>>>>>>>");
	for (auto &t: tests()){
		msg_write("== " + t.name + " ==");
		try{
			t.f();
			msg_write("  ok");
		}catch(Failure &e){
			msg_error(e.message());
		}
	}
}

void UnitTest::sleep(float t)
{
	hui::Timer timer;
	while (timer.peek() < t){
		hui::Application::do_single_main_loop();
		usleep(1000);
	}
}

#include "TestAudioBuffer.h"
#include "TestRhythm.h"
#include "TestStreams.h"
#include "TestThreads.h"
#include "TestMidiPreview.h"
#include "TestPlugins.h"

void UnitTest::run_all(const string &filter)
{
	Array<UnitTest*> tests;
	tests.add(new TestAudioBuffer);
	tests.add(new TestRhythm);
	tests.add(new TestThreads);
	tests.add(new TestStreams);
	tests.add(new TestMidiPreview);
	tests.add(new TestPlugins);

	for (auto *t: tests)
		if (filter.num == 0 or filter.find(t->name) >= 0)
			t->run();

	for (auto *t: tests)
		delete t;


}
