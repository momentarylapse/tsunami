/*
 * AudioScaleDialog.h
 *
 *  Created on: 2 Dec 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_AUDIOSCALEDIALOG_H_
#define SRC_VIEW_DIALOG_AUDIOSCALEDIALOG_H_

#include "../../lib/hui/hui.h"

namespace tsunami {

namespace BufferInterpolator {
	enum class Method;
}

class AudioScaleDialog : public hui::Dialog {
public:
	AudioScaleDialog(hui::Window *root, int original_size);

	int original_size;
	struct Data {
		int new_size;
		float pitch_scale;
		BufferInterpolator::Method method;
	};
	Data data;
	base::promise<Data> _promise;

	void update();

	void on_samples();
	void on_factor();
	void on_pitch_shift();
	void on_pitch_shift_semitones();

	void on_ok();
	void on_close();

	static base::future<Data> ask(hui::Window *root, int original_size);
};

}

#endif /* SRC_VIEW_DIALOG_AUDIOSCALEDIALOG_H_ */
