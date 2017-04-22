/*
 * SampleScaleDialog.h
 *
 *  Created on: 22.04.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_SAMPLESCALEDIALOG_H_
#define SRC_VIEW_DIALOG_SAMPLESCALEDIALOG_H_

#include "../../lib/hui/hui.h"

class Sample;

class SampleScaleDialog : public HuiDialog
{
public:
	SampleScaleDialog(HuiWindow *root, Sample *s);
	virtual ~SampleScaleDialog();

	Sample *sample;
	int new_size;

	void update(int mode = -1);

	void onSamples();
	void onFactor();
	void onSampleRate();
	void onSampleRateInv();

	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_SAMPLESCALEDIALOG_H_ */
