/*
 * TestStreams.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#include "TestStreams.h"
#include "../lib/file/msg.h"
#include "../lib/math/math.h"
#include "../Device/OutputStream.h"
#include "../Module/Audio/AudioSource.h"
#include "../Session.h"

TestStreams::TestStreams() : UnitTest("Streams")
{
}

Array<UnitTest::Test> TestStreams::tests()
{
	Array<Test> list;
	list.add(Test("output", TestStreams::test_output_stream));
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
	auto *stream = new OutputStream(Session::GLOBAL, source->out);

	msg_write("play");
	stream->play();
	sleep(1);
	msg_write("stop");
	stream->stop();
	sleep(1);
	msg_write("play");
	stream->play();
	sleep(1);
	msg_write("stop");
	stream->stop();
	delete(stream);
	delete(source);

}

