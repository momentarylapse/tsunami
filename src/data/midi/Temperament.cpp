/*
 * Temperament.cpp
 *
 *  Created on: 26 Mar 2022
 *      Author: michi
 */

#include "Temperament.h"
#include "../../lib/math/math.h"
#include <math.h>

/*Temperament::Temperament() {
}*/

bool Temperament::operator==(const Temperament &t) const {
	const float EPSILON = 0.01f;
	for (int p=0; p<MAX_PITCH; p++)
		if (fabs(freq[p] - t.freq[p]) > EPSILON)
			return false;
	return true;
}

bool Temperament::is_default() const {
	return *this == create_default();
}

bool Temperament::has_equal_octaves() const {
	const float EPSILON = 0.01f;
	for (int p=12; p<MAX_PITCH; p++)
		if (fabs(freq[p] - freq[p % 12] * pow(2.0f, float(p/12))) > 0.01f)
			return false;
	return true;
}



Temperament Temperament::create_default() {
	return create_equal12(MIDDLE_A, pitch_to_freq(MIDDLE_A));
}


Temperament Temperament::create_from_factor(int pitch_ref, float freq_ref, float factor[12]) {
	Temperament t;
	for (int i=0; i<MAX_PITCH; i++) {
		int rel = loop(i - pitch_ref, 0, 12);
		t.freq[i] = freq_ref * factor[rel] * pow(2.0f, float(i - rel - pitch_ref) / 12.0f);
	}
	return t;
};

Temperament Temperament::create_meantone(int pitch_ref, float freq_ref, float x) {
	float factor[12] = {1.0f, 8.0f/pow(x,5.0f), x*x/2, 4.0f/pow(x,3.0f), pow(x,4.0f)/4.0f, 2.0f/x, pow(x,6.0f)/8.0f, x, 8.0f/pow(x,4.0f), pow(x,3.0f)/2.0f, 4.0f/x/x, pow(x,5.0f)/4.0f};
	return create_from_factor(pitch_ref, freq_ref, factor);
};

Temperament Temperament::create_equal12(int pitch_ref, float freq_ref) {
	float factor = freq_ref / pitch_to_freq((float)pitch_ref);
	Temperament t;
	for (int p=0; p<MAX_PITCH; p++)
		t.freq[p] = pitch_to_freq((float)p) * factor;
	return t;
};

Temperament Temperament::create(TemperamentType type, int pitch_ref, float freq_ref) {
	if (type == TemperamentType::EQUAL_12) {
		return Temperament::create_equal12(pitch_ref, freq_ref);
	} else if (type == TemperamentType::MEANTONE_QUARTER_COMMA) {
		float x = pow(5.0f, 0.25f); // 5th
		// => major third = 5/4 = x^4/4
		return create_meantone(pitch_ref, freq_ref, x);
	} else if (type == TemperamentType::MEANTONE_THIRD_COMMA) {
		float x = pow(20.0f/6.0f, 1.0f/3.0f); // 5th
		// => minor third = 6/5 = 4/x^3
		return create_meantone(pitch_ref, freq_ref, x);
	} else if (type == TemperamentType::PYTHAGOREAN) {
		float x = 1.5f; // 5th
		float y = pow(x,4.0f) / 4.0f; // 3rd = x^4/4
		float factor[12] = {1.0f, 8.0f/pow(x,5.0f), x*x/2, 4.0f/pow(x,3.0f), y, 2.0f/x, pow(x,6.0f)/8.0f, x, 2.0f/y, pow(x,3.0f)/2.0f, 4.0f/x/x, pow(x,5.0f)/4.0f};
		return create_from_factor(pitch_ref, freq_ref, factor);
	} else if (type == TemperamentType::FIVE_LIMIT_DIATONIC_MAJOR) {
		float factor[12] = {1.0f, sqrt(9.0f/8.0f), 9.0f/8.0f, sqrt(45.0f/32.0f), 5.0f/4.0f, 4.0f/3.0f, sqrt(2.0f), 3.0f/2.0f, sqrt(15.0f/6.0f), 5.0f/3.0f, sqrt(75.0f/24.0f), 15.0f/8.0f};
		return create_from_factor(pitch_ref, freq_ref, factor);
	}
	return Temperament::create_default();
}

bool Temperament::guess_parameters(TemperamentType &type, int &pitch_ref, float &freq_ref) const {
	{
		type = TemperamentType::EQUAL_12;
		pitch_ref = MIDDLE_A;
		freq_ref = this->freq[pitch_ref];
		auto t = create_equal12(pitch_ref, freq_ref);
		if (t == *this)
			return true;
	}

	for (auto tt: Array<TemperamentType>({TemperamentType::MEANTONE_QUARTER_COMMA, TemperamentType::MEANTONE_THIRD_COMMA, TemperamentType::PYTHAGOREAN, TemperamentType::FIVE_LIMIT_DIATONIC_MAJOR})) {
		for (int i=0; i<12; i++) {
			type = tt;
			pitch_ref = MIDDLE_C + i;
			freq_ref = this->freq[pitch_ref];
			auto t = create(type, pitch_ref, freq_ref);
			if (t == *this)
				return true;
		}
	}

	return false;
}

