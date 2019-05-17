/*
 * TestStreams.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestStreams.h"
#include "../lib/file/msg.h"
#include "../lib/math/math.h"
#include "../Data/base.h"
#include "../Module/Audio/AudioSource.h"
#include "../Module/SignalChain.h"
#include "../Session.h"
#include "../Stream/AudioOutput.h"

TestStreams::TestStreams() : UnitTest("streams")
{
}

Array<UnitTest::Test> TestStreams::tests()
{
	Array<Test> list;
	list.add(Test("output", TestStreams::test_output_stream));
	list.add(Test("input", TestStreams::test_input_stream));
	return list;
}

class DebugAudioSource : public AudioSource
{
public:
	float phi, omega;
	DebugAudioSource()
	{
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

void TestStreams::test_output_stream()
{
	auto *source = new DebugAudioSource;
	auto *stream = new AudioOutput(Session::GLOBAL);
	stream->plug(0, source, 0);

	msg_write("play");
	stream->start();
	sleep(1);
	msg_write("stop");
	stream->stop();
	sleep(1);
	msg_write("play");
	stream->start();
	sleep(1);
	msg_write("stop");
	stream->stop();
	delete(stream);
	delete(source);

}

void TestStreams::test_input_stream()
{
	auto *chain = new SignalChain(Session::GLOBAL, "test");
	auto *a = chain->add(ModuleType::STREAM, "AudioInput");
	auto *b = chain->add(ModuleType::PLUMBING, "AudioSucker");
	chain->connect(a, 0, b, 0);

	msg_write("capture");
	chain->start();
	sleep(2);
	msg_write("stop");
	chain->stop();
	delete chain;

}

#endif
