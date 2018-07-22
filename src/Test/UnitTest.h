/*
 * UnitTest.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_TEST_UNITTEST_H_
#define SRC_TEST_UNITTEST_H_

#include "../lib/base/base.h"

class UnitTest
{
public:
	UnitTest();
	virtual ~UnitTest();

	virtual void run(){}

	//void assert(bool )

	class Failure : public Exception
	{
	public:
		Failure(const string &s) : Exception(s){}
	};
};

#endif /* SRC_TEST_UNITTEST_H_ */
