/*
 * UnitTest.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_TEST_UNITTEST_H_
#define SRC_TEST_UNITTEST_H_

#ifndef NDEBUG

#include "../lib/base/base.h"
#include <functional>

class AudioBuffer;
class Range;
class Path;

class UnitTest {
public:
	explicit UnitTest(const string &name);
	virtual ~UnitTest();

	string name;

	struct TestProtocoll {
		int num_tests_run = 0;
		int num_failed = 0;
	};

	class Test {
	public:
		string name;
		std::function<void()> f;
	};
	virtual Array<Test> tests() = 0;

	void run(const string &filter, TestProtocoll &protocoll);
	bool filter_match_group(const string &filter);
	bool filter_match(const string &filter, const string &test_name);

	//void assert(bool )

	class Failure : public Exception {
	public:
		Failure(const string &s) : Exception(s) {}
	};
	/*template<class T>
	static void assert_equal(const T &a, const T&b, const string &text)
	{
		if (a != b)
			throw Failure(text);
	}*/

	static AudioBuffer make_buf(const Array<float> &r, const Array<float> &l);

	static void assert_equal(float a, float b, float epsilon = 0.001f);
	static void assert_equal(const Array<int> &a, const Array<int> &b);
	static void assert_equal(const Array<float> &a, const Array<float> &b, float epsilon = 0.001f);
	static void assert_equal(const AudioBuffer &a, const AudioBuffer &b, float epsilon = 0.001f);
	static void assert_equal(const Range &a, const Range &b);
	static void assert_equal(const Array<Range> &a, const Array<Range> &b);
	static string ra2s(const Array<Range> &ra);

	static Array<UnitTest*> all();
	static void run_all(const string &filter);
	static void print_all_names();

	static void sleep(float t);
};
#endif

#endif /* SRC_TEST_UNITTEST_H_ */
