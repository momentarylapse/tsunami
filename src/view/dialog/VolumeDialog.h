/*
 * VolumeDialog.h
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_VOLUMEDIALOG_H_
#define SRC_VIEW_DIALOG_VOLUMEDIALOG_H_

#include "../../lib/hui/hui.h"

class VolumeDialog : hui::Dialog {
public:
	using Callback = std::function<void(float)>;
	VolumeDialog(hui::Window *parent, float value0, float min, float max, Callback cb);
	float result;
	float _min, _max, _min_db, _max_db;
	enum Mode {
		PERCENT,
		DB
	};
	Mode mode;
	void set_mode(Mode m);
	void set_spin(float f);
	void set_slider(float f);
	float get_spin();
	float get_slider();
	Callback cb;
	static bool aborted;
	static bool maximize;
	static void ask(hui::Window *parent, float value0, float min, float max, Callback cb);


	static constexpr float DB_MIN = -1000000;
	static constexpr float DB_MAX = 12;
	static constexpr float TAN_SCALE = 13.0f;

	static float db2slider(float db);
	static float slider2db(float val);
};



#endif /* SRC_VIEW_DIALOG_VOLUMEDIALOG_H_ */
