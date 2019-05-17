/*
 * TestRingBuffer.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestRingBuffer.h"
#include "../Data/Audio/RingBuffer.h"
#include <thread>
#include <math.h>

TestRingBuffer::TestRingBuffer() : UnitTest("ring-buffer")
{
}


Array<UnitTest::Test> TestRingBuffer::tests()
{
	Array<Test> list;
	list.add(Test("read-write", TestRingBuffer::test_read_write));
	list.add(Test("read-write-wrap", TestRingBuffer::test_read_write_wrap));
	list.add(Test("read-too-much", TestRingBuffer::test_read_too_much));
	list.add(Test("write-too-much-middle", TestRingBuffer::test_write_too_much_middle));
	list.add(Test("write-too-much-end", TestRingBuffer::test_write_too_much_end));
	list.add(Test("read-write-ref", TestRingBuffer::test_read_write_ref));
	list.add(Test("thread-safety", TestRingBuffer::test_thread_safety));
	return list;
}

void TestRingBuffer::test_read_write()
{
	RingBuffer ring(8);
	ring.write(make_buf({0,1,2,3}, {-1,-2,-3,-4}));

	if (ring.available() != 4)
		throw Failure("available != 4");


	ring.write(make_buf({10}, {20}));

	AudioBuffer buf;
	buf.resize(5);
	ring.read(buf);
	assert_equal(buf, make_buf({0,1,2,3,10}, {-1,-2,-3,-4,20}));
}

void TestRingBuffer::test_read_write_wrap()
{
	RingBuffer ring(6);
	ring.write(make_buf({0,1,2,3}, {-1,-2,-3,-4}));

	AudioBuffer buf;
	buf.resize(2);
	ring.read(buf);

	// write wrap
	ring.write(make_buf({10,20,30,40}, {-10,-20,-30,-40}));

	buf.resize(6);
	ring.read(buf);
	assert_equal(buf, make_buf({2,3,10,20,30,40}, {-3,-4,-10,-20,-30,-40}));
}

void TestRingBuffer::test_read_write_ref()
{
	RingBuffer ring(8);
	{
		AudioBuffer r;
		ring.write_ref(r, 4);
		r.set(make_buf({0,1,2,3}, {-1,-2,-3,-4}), 0);
		ring.write_ref_done(r);
	}

	if (ring.available() != 4)
		throw Failure("available != 4");


	{
		AudioBuffer r;
		ring.write_ref(r, 4);
		r.set(make_buf({10}, {20}), 0);
		ring.write_ref_done(r);
	}

	{
	AudioBuffer r;
	ring.read_ref(r, 5);
	assert_equal(r, make_buf({0,1,2,3,10}, {-1,-2,-3,-4,20}));
	ring.read_ref_done(r);
	}
}

void TestRingBuffer::test_read_too_much()
{
	RingBuffer ring(6);
	ring.write(make_buf({1,2}, {3,4}));

	AudioBuffer buf;
	buf.resize(4);
	int rr = ring.read(buf);
	if (rr != 2)
		throw Failure("read return != 2");

	assert_equal(buf, make_buf({1,2,0,0}, {3,4,0,0}));
}

void TestRingBuffer::test_write_too_much_end()
{
	RingBuffer ring(6);
	ring.write(make_buf({0,1,2,3}, {-1,-2,-3,-4}));
	/*{
		AudioBuffer r;
		ring.write_ref(r, 4);
		assert_equal(r, make_buf({0,0}, {0,0}));
		ring.write_ref_done(r);
	}*/
	int rr = ring.write(make_buf({10,20,30,40}, {-10,-20,-30,-40}));
	if (rr != 1)
		throw Failure("write a return != 1 .." + i2s(rr));


	{
		AudioBuffer r;
		ring.read_ref(r, 5);
		assert_equal(r, make_buf({0,1,2,3,10}, {-1,-2,-3,-4,-10}));
		ring.read_ref_done(r);
	}
}

void TestRingBuffer::test_write_too_much_middle()
{
	RingBuffer ring(6);
	ring.write(make_buf({0,1,2,3}, {-1,-2,-3,-4}));
	{
		AudioBuffer r;
		ring.read_ref(r, 1);
		ring.read_ref_done(r);
	}
	int rr = ring.write(make_buf({10,20,30,40}, {-10,-20,-30,-40}));
	if (rr != 3)
		throw Failure("write a return != 3 .." + i2s(rr));
	if (ring.available() != 6)
		throw Failure("available != 6...");

	AudioBuffer buf;
	buf.resize(6);
	rr = ring.read(buf);
	if (rr != 6)
		throw Failure("read return != 6...");
	assert_equal(buf, make_buf({1,2,3,10,20,30}, {-2,-3,-4,-10,-20,-30}));
}

void TestRingBuffer::test_thread_safety()
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
			if (read_count > write_count)
				throw Failure("read > write");
		}
	});

	consumer.join();
	producer.join();

	if (errors > 0)
		throw Failure(format("%d errors after %d samples", errors, total_samples));
}

#endif
