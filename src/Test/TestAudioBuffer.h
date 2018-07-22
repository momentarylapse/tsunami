/*
 * TestAudioBuffer.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_TEST_TESTAUDIOBUFFER_H_
#define SRC_TEST_TESTAUDIOBUFFER_H_

#include "UnitTest.h"

class TestAudioBuffer : public UnitTest
{
public:
	TestAudioBuffer();
	virtual ~TestAudioBuffer();
	void run() override;

	void test_ring_buffer_thread_safety();
};

#endif /* SRC_TEST_TESTAUDIOBUFFER_H_ */
