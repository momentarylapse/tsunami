/*
 * VolumeControl.h
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_VOLUMECONTROL_H_
#define SRC_VIEW_HELPER_VOLUMECONTROL_H_

#include "../../lib/base/base.h"
#include <functional>

namespace hui {
class Panel;
}

class VolumeControl {
public:
	using Callback = std::function<void(float)>;

	VolumeControl(hui::Panel* panel, const string& id_slider, const string& id_spin, const string& id_unit, Callback cb);

	void set(float f);
	float get() const;

	void set_range(float min, float max);

private:
	hui::Panel* panel;
	string id_slider, id_spin, id_unit;

	Callback cb;

	float value;
	float _min, _max, _min_db, _max_db;
	enum Mode {
		PERCENT,
		DB
	};
	Mode mode;
	void set_mode(Mode m);
	void set_spin(float f);
	void set_slider(float f);
	float get_spin() const;
	float get_slider() const;


	static constexpr float DB_MIN = -1000000;
	static constexpr float DB_MAX = 12;
	static constexpr float TAN_SCALE = 13.0f;

	static float db2slider(float db);
	static float slider2db(float val);
};

#endif /* SRC_VIEW_HELPER_VOLUMECONTROL_H_ */
