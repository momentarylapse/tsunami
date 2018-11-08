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

class SampleScaleDialog : public hui::Dialog
{
public:
	SampleScaleDialog(hui::Window *root, Sample *s);
	virtual ~SampleScaleDialog();

	Sample *sample;
	int new_size;

	void update(int mode = -1);

	void on_samples();
	void on_factor();
	void on_sample_rate();
	void on_sample_rate_inv();

	void on_ok();
	void on_close();
};

#endif /* SRC_VIEW_DIALOG_SAMPLESCALEDIALOG_H_ */
