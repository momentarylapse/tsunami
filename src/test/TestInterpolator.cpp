/*
 * TestInterpolator.cpp
 *
 *  Created on: 06.06.2019
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestInterpolator.h"
#include "../processing/audio/BufferInterpolator.h"
#include "../processing/audio/BufferPitchShift.h"

namespace tsunami {

TestInterpolator::TestInterpolator() : UnitTest("interpolator")
{
}


Array<UnitTest::Test> TestInterpolator::tests() {
	Array<Test> list;
	list.add({"linear-short", TestInterpolator::test_linear_short});
	list.add({"linear", TestInterpolator::test_linear});
	list.add({"cubic", TestInterpolator::test_cubic});
	list.add({"fourier", TestInterpolator::test_fourier});
	list.add({"operator", TestInterpolator::test_operator});
	list.add({"pitch-operator", TestInterpolator::test_pitch_operator});
	return list;
}

void TestInterpolator::test_linear_short() {
	Array<float> in = {1.0, 2.0};
	Array<float> out;
	out.resize((in.num - 1) * 4 + 1);
	BufferInterpolator::interpolate_channel_linear(in, out);
	assert_equal(out, {1.0, 1.25, 1.50, 1.75, 2.0});


	in = {1.0};
	out.resize(3);
	BufferInterpolator::interpolate_channel_linear(in, out);
	assert_equal(out, {1.0, 1.0, 1.0});
}

void TestInterpolator::test_linear() {
	Array<float> in = {1, 2, 0, 4};
	Array<float> out;
	out.resize((in.num - 1) * 2 + 1);

	BufferInterpolator::interpolate_channel_linear(in, out);
	assert_equal(out, {1, 1.5, 2, 1, 0, 2, 4});
}

void TestInterpolator::test_cubic() {
	Array<float> in = {0,0,1,0,2}, out;
	out.resize((in.num-1)*4+1);
	BufferInterpolator::interpolate_channel_cubic(in, out);
	assert_equal(out, {0.000000, -0.023438, -0.062500, -0.070312, 0.000000, 0.226562, 0.562500, 0.867188, 1.000000, 0.820312, 0.437500, 0.085938, 0.000000, 0.382812, 1.062500, 1.710938, 2.000000});

	for (int i=0; i<in.num; i++)
		assert_equal(in[i], out[i * 4]);
}

void TestInterpolator::test_fourier() {
}

void TestInterpolator::test_operator() {
	BufferInterpolator::Operator op;
	op.method = BufferInterpolator::Method::Linear;
	op.reset(2.0f);

	AudioBuffer buf(1024*8, 2);
	assert_equal(op.process(buf).length, 16384);
	assert_equal(op.process(buf).length, 16384);
}

void TestInterpolator::test_pitch_operator() {
	BufferPitchShift::Operator op;
	op.reset(2.0f, BufferInterpolator::Method::Linear, 1.0f);

	AudioBuffer buf(1024*8, 2);
	assert_equal(op.process(buf).length, 14336);
	assert_equal(op.process(buf).length, 16384);
}

}

#endif

