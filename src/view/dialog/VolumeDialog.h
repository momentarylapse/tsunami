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

class VolumeControl;

class VolumeDialog : obs::Node<hui::Dialog> {
public:
	using Callback = std::function<void(float)>;
	VolumeDialog(hui::Window *parent, float value0, float min, float max, Callback cb);

	owned<VolumeControl> volume_control;
	float result;
	Callback cb;

	static bool aborted;
	static bool maximize;
	static void ask(hui::Window *parent, float value0, float min, float max, Callback cb);
};



#endif /* SRC_VIEW_DIALOG_VOLUMEDIALOG_H_ */
