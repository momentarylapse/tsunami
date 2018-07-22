/*
 * UnitTest.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_TEST_UNITTEST_H_
#define SRC_TEST_UNITTEST_H_

#include "../lib/base/base.h"
#include <functional>

class UnitTest
{
public:
	UnitTest(const string &name);
	virtual ~UnitTest();

	string name;

	class Test
	{
	public:
		string name;
		std::function<void()> f;
		Test(){}
		Test(const string &_name, std::function<void()> _f)
		{ name = _name; f = _f; }
	};
	virtual Array<Test> tests() = 0;

	void run();

	//void assert(bool )

	class Failure : public Exception
	{
	public:
		Failure(const string &s) : Exception(s){}
	};

	static void run_all(const string &filter);

	static void sleep(float t);
};

#endif /* SRC_TEST_UNITTEST_H_ */
