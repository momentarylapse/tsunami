/*
 * TestThreads.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#include "TestThreads.h"
#include "../lib/file/msg.h"
#include "../lib/hui/hui.h"
#include "../lib/threads/Thread.h"

TestThreads::TestThreads() : UnitTest("threads")
{
}

Array<UnitTest::Test> TestThreads::tests()
{
	Array<Test> list;
	list.add(Test("safety", TestThreads::test_thread_safety));
	return list;
}

void TestThreads::test_thread_safety()
{
	class DebugThread : public Thread
	{
		void on_run() override
		{
			for (int i=0; i<10; i++)
			{
				hui::Sleep(0.1f);
			}
		}

	};
	DebugThread *t = new DebugThread;

	t->run();
	while (!t->is_done())
		sleep(0.01f);
	t->join();
	delete t;

}

