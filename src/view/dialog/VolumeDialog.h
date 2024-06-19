/*
 * VolumeDialog.h
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_VOLUMEDIALOG_H_
#define SRC_VIEW_DIALOG_VOLUMEDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../lib/pattern/Observable.h"

namespace tsunami {

class VolumeControl;

class VolumeDialog : obs::Node<hui::Dialog> {
public:
	VolumeDialog(hui::Window *parent, float value0, float min, float max);

	owned<VolumeControl> volume_control;
	float result;
	base::promise<float> _promise;

	static bool maximize;
	static base::future<float> ask(hui::Window *parent, float value0, float min, float max);
};

}

#endif /* SRC_VIEW_DIALOG_VOLUMEDIALOG_H_ */
