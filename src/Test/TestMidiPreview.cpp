/*
 * TestMidiPreview.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestMidiPreview.h"
#include "../lib/file/msg.h"
#include "../View/Helper/MidiPreview.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Session.h"
#include <thread>

TestMidiPreview::TestMidiPreview() : UnitTest("midi-preview")
{
}



Array<UnitTest::Test> TestMidiPreview::tests()
{
	Array<Test> list;
	list.add(Test("preview", TestMidiPreview::test_preview));
	return list;
}

void TestMidiPreview::test_preview()
{
	auto *synth = CreateSynthesizer(Session::GLOBAL, "");
	auto *preview = new MidiPreview(Session::GLOBAL, synth);
	msg_write("start");
	preview->start({64}, 1.0f, 1.0f);
	sleep(2.0f);

	msg_write("start");
	preview->start({64}, 1.0f, 1.0f);
	sleep(0.3f);
	msg_write("start");
	preview->start({68}, 1.0f, 1.0f);
	sleep(0.3f);
	msg_write("end");
	preview->end();
	sleep(0.3f);
	delete preview;
}

#endif
