/*
 * TestAudioBuffer.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestAudioBuffer.h"
#include "../Data/Audio/RingBuffer.h"
#include "../lib/file/msg.h"
#include <thread>
#include <math.h>

TestAudioBuffer::TestAudioBuffer() : UnitTest("audio-buffer")
{
}


Array<UnitTest::Test> TestAudioBuffer::tests()
{
	Array<Test> list;
	list.add(Test("ring-buffer-thread-safety", TestAudioBuffer::test_ring_buffer_thread_safety));
	return list;
}

void TestAudioBuffer::test_ring_buffer_thread_safety()
{
	RingBuffer ring(2000);//1<<20);
	int chunk_size = 1000;
	int count = 100;
	std::atomic<int> read_count(0), write_count(0);

	std::thread producer([&]{
		int x = 0;
		for (int i=0; i<count; i++){
			//printf("wa %d\n", ring.writable_size());
			while (ring.writable_size() < chunk_size){}
			//printf("w %d\n", i);
			AudioBuffer b;
			write_count ++;
			ring.write_ref(b, chunk_size);
			for (float &f: b.c[0])
				f = sin((float)x ++);
			ring.write_ref_done(b);
		}
	});
	int total_samples = 0;
	int errors = 0;
	std::thread consumer([&]{
		for (int i=0; i<count; i++){
			//printf("ra %d\n", ring.available());
			while (ring.available() < chunk_size){}
			AudioBuffer b;
			//printf("r %d\n", i);
			ring.read_ref(b, chunk_size);
			for (float &f: b.c[0])
				if (f != sin((float)total_samples ++))
					errors ++;
			ring.read_ref_done(b);
			read_count ++;
			if (read_count > write_count){
				msg_error("read > write");
				return;
			}
		}
	});

	consumer.join();
	producer.join();

	if (errors > 0)
		throw Failure(format("%d errors after %d samples", errors, total_samples));
}

#endif
