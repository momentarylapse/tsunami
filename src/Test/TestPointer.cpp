/*
 * TestPointer.cpp
 *
 *  Created on: Oct 4, 2020
 *      Author: michi
 */
#ifndef NDEBUG

#include "TestPointer.h"
#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/file/msg.h"




class X : public Sharable<VirtualBase> {
	string s;
public:
	X() {
		instance_count ++;
		msg_write("X.init " + p2s(this));
		s = "hallo";
	}
	~X() {
		msg_write("X.del " + p2s(this));
		instance_count --;
	}
	void f() {
		msg_write(s);
	}

	static int instance_count;
};
int X::instance_count = 0;


TestPointer::TestPointer() : UnitTest("pointer") {}


Array<UnitTest::Test> TestPointer::tests() {
	Array<Test> list;
	list.add({"owned", test_owned});
	list.add({"shared", test_shared});
	list.add({"owned-array", test_owned_array});
	list.add({"shared-array", test_shared_array});
	list.add({"shared-array-set-array", test_shared_array_set_array});
	list.add({"shared-array-set-shared-array", test_shared_array_set_shared_array});
	return list;
}

void TestPointer::test_owned() {
	{
		auto own = owned<X>(new X());
		assert_equal(X::instance_count, 1);
		auto x = std::move(own);
		assert_equal(X::instance_count, 1);

		X *p = nullptr;
		msg_write(x == p);
	}
	assert_equal(X::instance_count, 0);
}


void TestPointer::test_shared() {
	{
		shared<X> a = new X();
		assert_equal(X::instance_count, 1);
		{
			auto b = a;
			assert_equal(X::instance_count, 1);
			b->f();
		}
		assert_equal(X::instance_count, 1);
	}
	assert_equal(X::instance_count, 0);
}

void TestPointer::test_owned_array() {
	{
		owned_array<X> own;
		own.add(new X());
		own.add(new X());
		assert_equal(X::instance_count, 2);
		//auto x = std::move(own);
		//x->f();
	}
	assert_equal(X::instance_count, 0);
}

void TestPointer::test_shared_array() {
	{
		shared_array<X> a = {new X(), new X()};
		assert_equal(X::instance_count, 2);
		a.add(new X());
		a.add(new X());
		assert_equal(X::instance_count, 4);

		{
			auto b = a;
			assert_equal(X::instance_count, 4);
		}
		assert_equal(X::instance_count, 4);

		{
			auto b = a;
			assert_equal(X::instance_count, 4);
			a.clear();
			assert_equal(X::instance_count, 4);
		}
		assert_equal(X::instance_count, 0);
	}
	assert_equal(X::instance_count, 0);
}

void TestPointer::test_shared_array_set_shared_array() {
	{
		shared_array<X> a = {new X(), new X(), new X(), new X()};
		assert_equal(X::instance_count, 4);

		{
			msg_write("a1");
			shared_array<X> b = {new X(), new X()};
			assert_equal(X::instance_count, 6);
			msg_write("a2");
			a = b;
			msg_write("a3");
			assert_equal(X::instance_count, 2);
			msg_write("a3b");
		}
		msg_write("a4");
		assert_equal(X::instance_count, 2);
	}
	msg_write("a5");
	assert_equal(X::instance_count, 0);
}

void TestPointer::test_shared_array_set_array() {
	{
		shared_array<X> a = {new X(), new X(), new X(), new X()};
		assert_equal(X::instance_count, 4);

		{
			msg_write("a0");
			Array<X*> b = {new X(), new X()};
			msg_write("a1");
			assert_equal(X::instance_count, 6);
			msg_write("a1b");
			a = b;
			msg_write("a2");
			assert_equal(X::instance_count, 2);
		}
		msg_write("a3");
		assert_equal(X::instance_count, 2);
	}
	msg_write("a4");
	assert_equal(X::instance_count, 0);
}


#endif
