/*
 * TestAudioBuffer.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#include "TestAudioBuffer.h"
#include "../Data/Audio/RingBuffer.h"
#include "../lib/file/msg.h"
#include <thread>

TestAudioBuffer::TestAudioBuffer()
{
}

TestAudioBuffer::~TestAudioBuffer()
{
}

void TestAudioBuffer::run()
{
	msg_write("ring_buffer_thread_safety");
	try{
		test_ring_buffer_thread_safety();
		msg_write("  ok");
	}catch(Exception &e){
		msg_error(e.message());
	}
}

void TestAudioBuffer::test_ring_buffer_thread_safety()
{
	RingBuffer ring(2000);//1<<20);
	int chunk_size = 1000;
	int count = 10;
	std::atomic<int> read_count(0), write_count(0);

	std::thread producer([&]{
		for (int i=0; i<count; i++){
			//printf("wa %d\n", ring.writable_size());
			while (ring.writable_size() < chunk_size){}
			//printf("w %d\n", i);
			AudioBuffer b;
			write_count ++;
			ring.write_ref(b, chunk_size);
			ring.write_ref_done(b);
		}
	});
	std::thread consumer([&]{
		for (int i=0; i<count; i++){
			//printf("ra %d\n", ring.available());
			while (ring.available() < chunk_size){}
			AudioBuffer b;
			//printf("r %d\n", i);
			ring.read_ref(b, chunk_size);
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
}

