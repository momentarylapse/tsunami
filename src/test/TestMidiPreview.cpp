/*
 * TestMidiPreview.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestMidiPreview.h"
#include "../lib/file/msg.h"
#include "../view/helper/MidiPreview.h"
#include "../module/synthesizer/Synthesizer.h"
#include "../module/midi/MidiPreviewSource.h"
#include "../Session.h"
#include <thread>

TestMidiPreview::TestMidiPreview() : UnitTest("midi-preview") {}



Array<UnitTest::Test> TestMidiPreview::tests() {
	Array<Test> list;
	list.add({"preview", TestMidiPreview::test_preview});
	list.add({"preview-source", TestMidiPreview::test_preview_source});
	return list;
}

void TestMidiPreview::test_preview() {
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

void TestMidiPreview::test_preview_source() {
	auto *synth = CreateSynthesizer(Session::GLOBAL, "");
	auto *preview = new MidiPreview(Session::GLOBAL, synth);
	auto source = preview->source;
	auto chain = preview->chain;
	chain->set_buffer_size(4);

	msg_write("start");
	source->start({64}, 10, 1.0f);
	msg_write(chain->do_suck());
	source->end();
	msg_write(chain->do_suck());
	msg_write(chain->do_suck());

	delete preview;
}

#endif
