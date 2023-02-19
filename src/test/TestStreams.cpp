/*
 * TestStreams.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestStreams.h"
#include "../lib/os/msg.h"
#include "../lib/math/math.h"
#include "../data/base.h"
#include "../module/audio/AudioSource.h"
#include "../module/SignalChain.h"
#include "../Session.h"
#include "../device/stream/AudioOutput.h"

TestStreams::TestStreams() : UnitTest("streams") {
}

Array<UnitTest::Test> TestStreams::tests() {
	Array<Test> list;
	list.add({"output", TestStreams::test_output_stream});
	list.add({"input", TestStreams::test_input_stream});
	return list;
}

class DebugAudioSource : public AudioSource {
public:
	float phi, omega;
	DebugAudioSource() {
		omega = 2*pi * 440.0f / (float)DEFAULT_SAMPLE_RATE;
		phi = 0;
	}
	virtual int _cdecl read(AudioBuffer &buf){
		printf("read %d\n", buf.length);
		for (int i=0; i<buf.length; i++){
			buf.c[0][i] = sin(phi);
			buf.c[1][i] = sin(phi);
			phi += omega;
		}
		return buf.length;
	}
};

void TestStreams::test_output_stream() {
	auto chain = ownify(new SignalChain(Session::GLOBAL, "test"));
	auto source = chain->_add(new DebugAudioSource);
	auto stream = chain->add(ModuleCategory::STREAM, "AudioOutput");
	chain->connect(source.get(), 0, stream.get(), 0);

	event("play");
	chain->start();
	sleep(1);
	event("stop");
	chain->stop();
	sleep(1);
	event("play");
	chain->start();
	sleep(1);
	event("stop");
	chain->stop();

}

void TestStreams::test_input_stream() {
	auto chain = ownify(new SignalChain(Session::GLOBAL, "test"));
	auto a = chain->add(ModuleCategory::STREAM, "AudioInput");
	auto b = chain->add(ModuleCategory::PLUMBING, "AudioSucker");
	chain->connect(a.get(), 0, b.get(), 0);

	event("capture");
	chain->start();
	sleep(2);
	event("stop");
	chain->stop();

}

#endif
